#include <list>
#include "constants.h"
#include "Bitboard.h"
#include "PieceNode.h"



void PieceNode::reposition(list <PieceNode*> &newNeighbors, BitboardContainer &newBitboard){
	remove();
	insert(newNeighbors, newBitboard);
}

void PieceNode::insert( list <PieceNode*> &newNeighbors, BitboardContainer &newBitboard) {
	for (auto otherNode: newNeighbors) {
		otherNode -> neighbors.push_front(this);	
		neighbors.push_front(otherNode);
	}
	auto LSB = newBitboard.getLeastSignificantBit();
	boardIndex = LSB.first;
	location = LSB.second;
}

void PieceNode::remove() {
	for (auto otherNode: neighbors) {
		otherNode -> neighbors.remove(this);
	}
	neighbors.clear();
	boardIndex = -1;
	location = 0;
}

void PieceNode::print() {
	cout << "boardIndex " <<  boardIndex << " location" << location << endl;
}
