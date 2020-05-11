#pragma once
#include <vector>
#include <list>
#include "PieceNode.h"
#include <unordered_map>
#include <stack>
#include <unordered_set>
#include <functional>
using namespace std;

class PieceGraph {
		//the pieceGraph assumes that the graph is connected
		unordered_set <PieceNode*> articulationNodes;

		//assumes that this is updated before every insert or remove operation
		Bitboard allPieces;

		unordered_map <int, PieceNode*> bitboardHashTable;
		//no adjecency list needed; it is stored within the nodes themselves
		//list <list<PieceNode>> adjList;
		
	public:

		~PieceGraph() { destroy();}

		void insert(Bitboard&);
		void remove(Bitboard&);
		void reposition(Bitboard&, Bitboard&);

		void getArticulationNodes(PieceNode*, int&, unordered_set<PieceNode*>&);
		void checkArticulationRoot(PieceNode*);

		bool checkBiDirectional(Bitboard, Bitboard);

		bool isTopOfStack();

		Bitboard getPinnedPieces();

		void destroy();

		void initializeTo (const PieceGraph& other) {
			for (auto element: other.bitboardHashTable) {
				Bitboard bitboard;
				bitboard.setBoard(element.second->boardIndex, element.second -> location);
				insert(bitboard);
			}
		}

		void DFS ( PieceNode*, unordered_set<PieceNode*>&);
		unordered_set<PieceNode*> DFS ();

		void operator=(const PieceGraph& other){
			destroy();
			initializeTo(other);
		}
};
