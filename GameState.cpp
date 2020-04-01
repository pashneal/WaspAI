#include "GameState.h"
#include <vector>
#include <set>

using namespace std;

GameState::GameState () {

	problemNodeContainer.allPieces = &allPieces;
	moveGenerator.allPieces = &allPieces;
	moveGenerator.problemNodes = &problemNodeContainer;

}

MoveInfo GameState::insertPiece(BitboardContainer& bitboard, PieceName name) {

	MoveInfo moveInfo;

	moveInfo.pieceName = name;

	if (bitboard.count() == 0) return moveInfo;

	pieceGraph.insert(bitboard);
	problemNodeContainer.insertPiece(bitboard);

	if (allPieces.containsAny(bitboard)) {
		stackHashTable[bitboard.hash()].push({turnColor, name});
		upperLevelPieces.xorWith(bitboard);
	}

	allPieces.unionWith(bitboard);
	getPieces(name) -> unionWith(bitboard);
	getPieces(turnColor) -> unionWith(bitboard);
	
	moveInfo.newPieceLocation.initializeTo(bitboard);

	return moveInfo;
}

MoveInfo GameState::movePiece(BitboardContainer& oldBitboard, BitboardContainer& newBitboard,
							  PieceName name) {


	MoveInfo moveInfo = insertPiece(newBitboard, name);
	int bitHash = oldBitboard.hash();

	if (stackHashTable.find(bitHash) != stackHashTable.end()) { 
		if (stackHashTable[bitHash].top().first == turnColor &&
		    stackHashTable[bitHash].top().second == name) 
		{

			stackHashTable[bitHash].pop();
			
			if (!(stackHashTable[bitHash].size())) {
				stackHashTable.erase(bitHash);
				upperLevelPieces.xorWith(oldBitboard);
			}
		}

	} else {

		problemNodeContainer.removePiece(oldBitboard);
		pieceGraph.remove(oldBitboard);
		allPieces.xorWith(oldBitboard);
		getPieces(name) -> xorWith(oldBitboard);
		getPieces(turnColor) -> xorWith(oldBitboard);
	}

	moveInfo.oldPieceLocation.initializeTo(oldBitboard);

	return moveInfo;
}

void GameState::undoMove(MoveInfo moveInfo) {
	changeTurnColor();
	movePiece(moveInfo.newPieceLocation, moveInfo.oldPieceLocation,
			  moveInfo.pieceName);
}

PieceColor GameState::checkVictory() {
	
	//assumes no draws are in board
	BitboardContainer queenCheck;

	queenCheck.initializeTo(queens);
	//check white queen
	queenCheck.intersectionWith(whitePieces);
	queenCheck = queenCheck.getPerimeter();
	queenCheck.intersectionWith(allPieces);
	if (queenCheck.count() == 6) return PieceColor::BLACK;

	queenCheck.initializeTo(queens);
	//check black queen
	queenCheck.intersectionWith(whitePieces);
	queenCheck = queenCheck.getPerimeter();
	queenCheck.intersectionWith(allPieces);
	if (queenCheck.count() == 6) return PieceColor::WHITE;

	return PieceColor::NONE;
}

bool GameState::checkDraw() {
	//TODO: draw by repetition OUTSIDE of GameState
	BitboardContainer queenCheck(queens);
	queenCheck = queenCheck.getPerimeter();
	queenCheck.intersectionWith(allPieces);

	BitboardContainer originalQueenPerimeter = queens.getPerimeter();
	return (queenCheck == originalQueenPerimeter);
}

inline BitboardContainer * GameState::getPieces() { 
	return &allPieces;
}

BitboardContainer * GameState::getPieces(PieceName name) {
	switch (name)   
	{
	case GRASSHOPPER: return &grasshoppers ;
	case QUEEN:       return &queens       ;
	case LADYBUG:     return &ladybugs     ;
	case PILLBUG:     return &pillbugs     ;
	case MOSQUITO:    return &mosquitoes   ;
	case BEETLE:      return &beetles      ;
	case ANT:         return &ants         ;
	case SPIDER:      return &spiders      ;
		break;
	}
}

BitboardContainer * GameState::getPieces(PieceColor color) {
	switch (color) {
		case WHITE: return &whitePieces;
		case BLACK: return &blackPieces;
		case NONE:  return &allPieces;
	}
}

void GameState::changeTurnColor() {

	if (turnColor == PieceColor::BLACK) {
		turnColor = PieceColor::WHITE;

	} else  {
		turnColor = PieceColor::BLACK;
	}
}

void GameState::destroy() {
	pieceGraph.destroy();
}

//no piece can move if it was moved last turn
//(that includes swaps)

//Pillbug swap and last move restriction;
//cannot swap out from up top hive or through gates
//blocked when covered
//can still use power if pinned

//beetle also can't move through upperLevelGates
//don't care about atop hive (MOVEGENERATOR)


//Mosquito movement ={
//only a bettle on the hive
//also can't move through upperLevel gates


//delayed legality check
