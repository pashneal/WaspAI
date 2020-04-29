#pragma once 
#include <list>
#include <unordered_map>
#include <unordered_set>
#include "Bitboard.h"
#include <stack>

using namespace std;

class ProblemNodeContainer {
		//TODO: this might be an optimization worth trying
		//unordered_map <int, list<Bitboard>> perimeterHashTable;
		
		unordered_map <int, list<Bitboard>> locationHashTable;

		unordered_set <int> problemNodeHashes;

		list <Bitboard> getProblemNodesAtLocation(int,unsigned long long);

		int hash(int, unsigned long long);

		void updateVisible(Bitboard&);
		void remove(Bitboard&);
		//requires that currentPiece is actally in *allPieces? (maybe?)
		void insert(Bitboard&);

	public:

		Bitboard *allPieces;
		Bitboard visibleProblemNodes;

		ProblemNodeContainer(){};
		ProblemNodeContainer(Bitboard*);

		bool problemNodeExists(Bitboard&);

		void insertPiece(Bitboard&);
		void removePiece(Bitboard&);

		void clear();

		void findAllProblemNodes();

		Bitboard getPerimeter(Bitboard&);

		bool contains(Bitboard&);
};

