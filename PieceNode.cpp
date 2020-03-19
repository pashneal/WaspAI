#include <list>
#include "contants.h"
#include "Bitboard.h"
#include "PieceNode.h"

PieceNode::PieceNode(int pieceNum) { 
	pieceNumber = pieceNum;
}

void PieceNode::shiftDirection(Direction dir) {
	bitboard.shiftDirection(dir);
}

void PieceNode::shiftDirection(Direction dir, int numTimes) {
	for (int i = 0 ; i < numTimes; i++) {
		bitboard.shiftDirection(dir);
	}
}

void PieceNode::reposition(list <PieceNode> &newNeighbors, BitboardContainer &newBitboard){
	for (auto otherNode: neighbors) {
		for (auto currentNode: (*otherNode).neighbors) {
			if (pieceNumber == (*currentNode).pieceNumber) {
				(*otherNode).neighbors.erase(currentNode);
				break;
			}
		}
	}
	neighbors.clear();
	insert(newNeighbors, newBitboard);
}

void PieceNode::insert( list <PieceNode> &newNeighbors, BitboardContainer &newBitboard) {
	for (auto otherNode: newNeighbors) {
		(*otherNode).neighbors.push_front(this);	
		neighbors.push_front(otherNode);
	}
	bitboard.initializeTo(newBitboard);
}

