#pragma once 
#include <list>
#include <unordered_map>
#include <unordered_set>
#include "Bitboard.h"

using namespace std;

class ProblemNodeContainer {

		BitboardContainer *allPieces;

		//TODO: this might be an optimization worth trying
		//unordered_map <int, list<BitboardContainer>> perimeterHashTable;
		
		unordered_map <int, list<BitboardContainer>> locationHashTable;

		unordered_set <int> problemNodeHashes;

		list <BitboardContainer> getProblemNodesAtLocation(int,unsigned long long);

		int hash(int, unsigned long long);
		int hash(BitboardContainer&);

		void updateVisible(BitboardContainer&);
		void remove(BitboardContainer&);
		//requires that a piece is actally in *allPieces
		void insert(BitboardContainer&);
		bool problemNodeExists(BitboardContainer&);

	public:

		BitboardContainer visibleProblemNodes;

		ProblemNodeContainer(BitboardContainer*);

		void insertPiece(BitboardContainer&);
		void removePiece(BitboardContainer&);

		void clear();

		void findAllProblemNodes();

		BitboardContainer getPerimeter(BitboardContainer&);
};

// gate (1) (2) 
// (1) -> allGatesContaining(1)j
