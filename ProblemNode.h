#pragma once 
#include <list>
#include <unordered_map>
#include <unordered_set>
#include "Bitboard.h"

using namespace std;

//TODO: fix this class. it is over engineered
//slightly better :}
class ProblemNodeContainer {

		BitboardContainer *allPieces;

		//TODO: this might be an optimization worth trying
		//unordered_map <int, list<BitboardContainer>> perimeterHashTable;
		
		//maps location -> all problemNodes containing that location
		unordered_map <int, list<BitboardContainer>> locationHashTable;

		unordered_set <int> problemNodeHashes;

		list <BitboardContainer> getProblemNodesAtLocation(int,unsigned long long);

		int hash(int, unsigned long long);
		int hash(BitboardContainer&);

		void updateVisible(BitboardContainer&);
		void remove(BitboardContainer&);
		void insert(BitboardContainer&);

	public:

		BitboardContainer visibleProblemNodes;

		ProblemNodeContainer(BitboardContainer*);

		void insertPiece(BitboardContainer&);
		void removePiece(BitboardContainer&);

		void clear();

		void findAllProblemNodes();
};

// gate (1) (2) 
// (1) -> allGatesContaining(1)j
