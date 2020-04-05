#pragma once 
#include <list>
#include <unordered_map>
#include <unordered_set>
#include "Bitboard.h"
#include <stack>

using namespace std;

class ProblemNodeContainer {
		//TODO: this might be an optimization worth trying
		//unordered_map <int, list<BitboardContainer>> perimeterHashTable;
		
		unordered_map <int, list<BitboardContainer>> locationHashTable;

		unordered_set <int> problemNodeHashes;

		list <BitboardContainer> getProblemNodesAtLocation(int,unsigned long long);

		int hash(int, unsigned long long);

		void updateVisible(BitboardContainer&);
		void remove(BitboardContainer&);
		//requires that currentPiece is actally in *allPieces? (maybe?)
		void insert(BitboardContainer&);

		BitboardContainer getLegalClimbs(BitboardContainer&, BitboardContainer&,
						  unordered_map < int , stack < pair <PieceColor, PieceName>>>);

	public:

		BitboardContainer *allPieces;
		BitboardContainer visibleProblemNodes;

		ProblemNodeContainer(){};
		ProblemNodeContainer(BitboardContainer*);

		bool problemNodeExists(BitboardContainer&);

		void insertPiece(BitboardContainer&);
		void removePiece(BitboardContainer&);

		void clear();

		void findAllProblemNodes();

		BitboardContainer getPerimeter(BitboardContainer&);

		bool contains(BitboardContainer&);
};

