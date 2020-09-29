#include "Weight.h"


void KillShotCountWeight::initialize(GameState* g) {
	Weight::initialize(g);
	watchPoints.clear();
	pinnedWatchPoints.clear();
	unpinnedWatchPoints.clear();

	//set watchpoints to queen and her surroudings
	//if there is no movement in these zones, the 
	//default value do not have to be recalculated
	queenCount = 0;
	for (auto& queen : parentGameState->queens.splitIntoBitboards()){
		//watch points for piece movements
		watchPoints.unionWith(queen);
		Bitboard queenPerimeter = queen.getPerimeter();
		watchPoints.unionWith(queenPerimeter);
		queenCount++;
		queenPerimeter.intersectionWith(parentGameState->allPieces);

		PieceColor queenColor = parentGameState->findBottomPieceColor(queen);
		//watch points for friendly piece mobility
		for (auto& piece: queenPerimeter.splitIntoBitboards()){

			if (parentGameState->findTopPieceColor(piece) == queenColor &&
			!parentGameState->upperLevelPieces.containsAny(piece)){

				if (parentGameState->pinned.containsAny(piece) ) {
					pinnedWatchPoints.unionWith(piece);
				} else {
					unpinnedWatchPoints.unionWith(piece);
				}
			}
		}
	}
	auto result = recalculate();
	scores[PieceColor::WHITE] = result[WHITE];
	scores[PieceColor::BLACK] = result[BLACK];
}

vector<int> KillShotCountWeight::recalculate() {
	vector<int> killShotCount{0,0};
	for (auto& queen : parentGameState->queens.splitIntoBitboards()){
	
		Bitboard queenPerimeter = queen.getPerimeter();
		PieceColor queenColor = parentGameState->findBottomPieceColor(queen);

		MoveGenerator moveGen(&parentGameState->allPieces);
		moveGen.setPieceStacks(&parentGameState->pieceStacks);
		moveGen.setUpperLevelPieces(&parentGameState->upperLevelPieces);

		//calculate and store default values
		queenPerimeter.intersectionWith(parentGameState->allPieces);
		for (auto& piece: queenPerimeter.splitIntoBitboards()){

			//if the pieceStack > 1, it takes too long to remove the piece
			if (parentGameState->upperLevelPieces.containsAny(piece)){
				killShotCount[queenColor]++;
				continue;
			}
			if (parentGameState->findTopPieceColor(piece) == queenColor){
				if (!parentGameState->pinned.containsAny(piece)){
					//if the unpinned friendly piece can move
					PieceName name = parentGameState->findTopPieceName(piece);
					moveGen.setGeneratingName(&name);
					moveGen.setGeneratingPieceBoard(&piece);
					if (moveGen.getMoves().count()){
						killShotCount[queenColor] += .16666;
						continue;
					}
				} 
			}
			killShotCount[queenColor]++;
		}
	}
	return killShotCount;
}

/*
 * Assumes that the player that is maximizing the score
 * is the one that just moved
 */
double KillShotCountWeight::evaluate(MoveInfo move){
	double result;
	Bitboard b(parentGameState->pinned);
	b.notIntersectionWith(pinnedWatchPoints);
	//see if necessary to recalculate weight
	if ( watchPoints.containsAny(move.newPieceLocation) ||
		 watchPoints.containsAny(move.oldPieceLocation) || 
		 unpinnedWatchPoints.containsAny(parentGameState->pinned) ||
		 b.count() ||
		 queenCount < 2)
	{
		auto killShotCounts = recalculate();
		result = killShotCounts[WHITE] - killShotCounts[BLACK];
	} else {
		result = scores[WHITE] - scores[BLACK];
	}

	//initially assumed maximizing player is WHITE
	//correct assumptions if necessary
	if (parentGameState->turnColor == WHITE) result = -result;
	
	return result*multiplier;
}

double PinnedWeight::evaluate(MoveInfo move){
	int result = 0;

	Bitboard whiteQueenPerimeter, blackQueenPerimeter;
	for (auto& piece: parentGameState->queens.splitIntoBitboards()){
		if (parentGameState->findBottomPieceColor(piece) == WHITE) 
			whiteQueenPerimeter = piece.getPerimeter();
		else 
			blackQueenPerimeter = piece.getPerimeter();
	}
	
	for (Bitboard& piece : parentGameState->allPieces.splitIntoBitboards()){
		//don't count piece if not pinned or if on top of hive
		if (parentGameState->upperLevelPieces.containsAny(piece)
			|| !(parentGameState->pinned.containsAny(piece)))  
		{	
			//don't count piece if pinned beside opposing queen
			if (parentGameState->findTopPieceColor(piece) == WHITE
				&& !blackQueenPerimeter.containsAny(piece))  
			{
				result++;
			} else if (parentGameState->findTopPieceColor(piece) == BLACK
				&& !whiteQueenPerimeter.containsAny(piece))  
			{
				result--;
			}
		}
	}

	if (parentGameState->turnColor == WHITE) 
		result = -result;

	return result * multiplier;
}

void PieceCountWeight::initialize(GameState * gameState){
	Weight::initialize(gameState);
	for (int i = PieceColor::WHITE; i < PieceColor::NONE; i++){
		for (auto element : gameState->unusedPieces[i]) {
			pieceCounts[i] = HivePLM[i][element.first] - element.second;
		}
	}
}

double PieceCountWeight::evaluate(MoveInfo move) {
	double result = pieceCounts[WHITE] - pieceCounts[BLACK];
	if (!(move.newPieceLocation == Bitboard()) && move.oldPieceLocation == Bitboard()) {
		if (parentGameState->findTopPieceColor(move.newPieceLocation) == WHITE)
			result++;
		else 
			result--;
	} 
	
	if (parentGameState->turnColor == WHITE) 
		result = -result;
	return result*multiplier;
}

double PinningPowerWeight::evaluate(MoveInfo move){
	Bitboard unpinned = parentGameState->pinned;
	unpinned.xorWith(parentGameState->allPieces);
	Bitboard pinningPieces;
	for (auto& testPiece : pinningPieces.splitIntoBitboards()){
		Bitboard testPiecePerimeter = testPiece.getPerimeter();
		testPiecePerimeter.intersectionWith(parentGameState->allPieces);
		testPiecePerimeter.notIntersectionWith(unpinned);
		if (testPiecePerimeter.count())
			pinningPieces.unionWith(testPiece);
	}
	return 0;
}


