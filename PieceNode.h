#pragma once
#include <list>
#include "Bitboard.h"
#include "constants.h"
#include <unordered_map>

using namespace std;

class PieceNode {
	public:	
		int boardIndex = -1;
		unsigned long long location = 0;

		list <PieceNode*> neighbors;	

		int visitedNum;
		int lowLink;

		PieceNode * parent;
		
		PieceNode(){};
		void shiftDirection(Direction, int);
		void shiftDirection(Direction);
		void reposition(list<PieceNode*>&, Bitboard&);
		void insert(list<PieceNode*>&, Bitboard&);
		void remove();
		void print();
		
		bool operator==(PieceNode &other) {
			return (boardIndex == other.boardIndex && 
					location   == other.location);
		}

};
