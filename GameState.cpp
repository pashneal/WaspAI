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

	pieceGraph.insert(bitboard);
	problemNodeContainer.insertPiece(bitboard);

	if (allPieces.containsAny(bitboard)) {
		stackHashTable[bitboard.hash()].push({turnColor, name});
	}
	allPieces.unionWith(bitboard);
	getPieces(name) -> unionWith(bitboard);
	getPieces(turnColor) -> unionWith(bitboard);
	
	moveInfo.pieceColor = turnColor;
	moveInfo.newPieceLocation.initializeTo(bitboard);
	moveInfo.pieceName = name;

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
			}
		}

	} else {
		allPieces.xorWith(oldBitboard);
		getPieces(name) -> xorWith(oldBitboard);
		getPieces(turnColor) -> xorWith(oldBitboard);
	}

	moveInfo.oldPieceLocation.initializeTo(oldBitboard);

	return moveInfo;
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
	}
}
