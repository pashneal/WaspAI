#include "Weight.h"

void KillShotCountWeight::initializeTo(GameState* g) {
	parentGameState = g;
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
	PieceColor maximizingColor = parentGameState->turnColor;
	if (maximizingColor == PieceColor::BLACK) result = -result;
	
	return result*multiplier;
}


