#pragma once
#include <list>
#include "Bitboard.h"
#include "constants.h"
#include <unordered_map>

using namespace std;

class PieceNode {
		BitboardContainer bitboard;
	public:	
		list <PieceNode*> neighbors;	

		bool visited = false;
		bool isEmpty = true;
		int visitedNum;
		int lowLink;
		PieceNode * parent;
		
		int pieceNumber;

		PieceNode() {};
		PieceNode(int, BitboardContainer);
		void shiftDirection(Direction, int);
		void shiftDirection(Direction);
		void reposition(list<PieceNode*>&, BitboardContainer&);
		void insert(list<PieceNode*>&, BitboardContainer&);
};
