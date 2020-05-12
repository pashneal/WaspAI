#include "Weight.h"

void KillShotCountWeight::initialize(GameState* g) {
	Weight::initialize(g);
	watchPoints.clear();
	//set watchpoints to queen and her surroudings
	//if there is no movement in these zones, the 
	//default value do not have to be recalculated
	queenCount = 0;
	for (auto& queen : parentGameState->queens.splitIntoBitboards()){
		watchPoints.unionWith(queen);
		Bitboard queenPerimeter = queen.getPerimeter();
		watchPoints.unionWith(queenPerimeter);
		queenCount++;
	}
	auto result = recalculate();
	queenKillShotCount[PieceColor::WHITE] = result.first;
	queenKillShotCount[PieceColor::BLACK] = result.second;
}

pair<int, int> KillShotCountWeight::recalculate() {
	pair<int,int> killShotCount{0,0};
	for (auto& queen : parentGameState->queens.splitIntoBitboards()){
	
		Bitboard queenPerimeter = queen.getPerimeter();

		//find color by looking at the bottom of stack 
		auto& stackOnQueen = parentGameState->pieceStacks[queen.hash()];
		PieceColor color = stackOnQueen.back().first;

		//calculate and store default values
		queenPerimeter.intersectionWith(parentGameState->allPieces);
		if (color == 0)
			killShotCount.first = queenPerimeter.count();
		else 
			killShotCount.second = queenPerimeter.count();
	}
	return killShotCount;
}

/*
 * Assumes that the player that is maximizing the score
 * is the one that just moved
 */
double KillShotCountWeight::evaluate(MoveInfo move){
	double result;
	//see if necessary to recalculate weight
	if ( watchPoints.containsAny(move.newPieceLocation) ||
		 watchPoints.containsAny(move.oldPieceLocation) || 
		 queenCount < 2)
	{
		auto killShotCounts = recalculate();
		result = killShotCounts.first - killShotCounts.second;
			
	} else {
		result = queenKillShotCount[WHITE] - queenKillShotCount[BLACK];
	}

	//initially assumed maximizing player is WHITE
	//correct assumptions if necessary
	if (parentGameState->turnColor == WHITE) result = -result;
	
	return result*multiplier;
}

double PinnedWeight::evaluate(MoveInfo move){
	int result = 0;

	for (Bitboard& piece : parentGameState->allPieces.splitIntoBitboards()){
		if (parentGameState->upperLevelPieces.containsAny(piece)
			|| !(parentGameState->pinned.containsAny(piece)))  
		{	
			if (parentGameState->findTopPieceColor(piece) == WHITE) 
				result++;
			else 
				result--;
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

void SimpleMoveCountWeight::initialize(GameState * gameState) {
	Weight::initialize(gameState);
	if (name == SPIDER || name == BEETLE ||name == QUEEN ||name == PILLBUG )
			for (Bitboard& piece : gameState->getPieces(name)->splitIntoBitboards()) {
				watchPoints.unionWith(piece);
				piece = piece.getPerimeter();
				watchPoints.unionWith(piece);
			}
	else {
			cout << "Cannot initialize SimpleMoveCount to given PieceName" << endl;
			throw 93;
	}
	moveGenerator.setPieceStacks(&gameState->pieceStacks);
	moveGenerator.setGeneratingName(&name);
	auto result = recalculate();
	moveCounts[WHITE] = result[WHITE];
	moveCounts[BLACK] = result[BLACK];
}

vector<int> SimpleMoveCountWeight::recalculate() {
	vector<int> result{0,0};
	for (Bitboard& piece : parentGameState->getPieces(name)->splitIntoBitboards()){
		//if a beetle, check whether the piece is on top of the hive
		if (name == BEETLE) {

			if (parentGameState->upperLevelPieces.containsAny(piece)){
				if (parentGameState->findTopPieceName(piece) != BEETLE)
					continue;
			} else {
				if (parentGameState->pinned.containsAny(piece))
					continue;
			}

		} else if (parentGameState->findTopPieceName(piece) != name ||
				   parentGameState->pinned.containsAny(piece)) {
			continue;
		}
		moveGenerator.setGeneratingPieceBoard(&piece);

		PieceColor color = parentGameState->findTopPieceColor(piece);
		result[color] += moveGenerator.getMoves().count();
	}
	return result;
}

double SimpleMoveCountWeight::evaluate(MoveInfo move){
	double result;
	//see if necessary to recalculate weight
	if ( watchPoints.containsAny(move.newPieceLocation) ||
		 watchPoints.containsAny(move.oldPieceLocation) )
	{
		vector<int> recalulatedMoveCounts = recalculate();
		result = recalulatedMoveCounts[WHITE] - recalulatedMoveCounts[BLACK];
			
	} else {
		result = moveCounts[WHITE] - moveCounts[BLACK];
	}

	//initially assumed maximizing player is WHITE
	//correct assumptions if necessary
	PieceColor maximizingColor = parentGameState->turnColor;
	if (maximizingColor == PieceColor::BLACK) result = -result;
	
	return result*multiplier;
}


