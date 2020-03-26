#pragma once 
#include <list>
#include <unordered_map>
#include "Bitboard.h"

using namespace std;

class ProblemNodeContainer {
		unordered_map <int, list<BitboardContainer> > problemNodeMap;
		unordered_map <int, list<BitboardContainer*>> problemNodeHashTable;
		unordered_map <int, list<BitboardContainer*>> gateHashTable;
		int hash(unsigned long long, int);
	public:
		void insert(BitboardContainer, BitboardContainer);
		void removeProblemNodesCausedByPiece(BitboardContainer&);
		void clear();
};
