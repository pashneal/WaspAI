#pragma once
#include <vector>
#include <list>
#include "PieceNode.h"
#include <unordered_map>

using namespace std;

class PieceGraph {
		//the pieceGraph assumes that the graph is connected
		set <PieceNode*> articulationNodes;
		list <PieceNode*> allPieceNodes;


		//assumes that this is updated before every insert or remove operation
		BitboardContainer allPieces;

		unordered_map <int, PieceNode*> bitboardHashTable;
		//no adjecency list needed; it is stored within the nodes themselves
		//list <list<PieceNode>> adjList;
	public:

		void insert(BitboardContainer&);
		void remove(BitboardContainer&);
		void reposition(BitboardContainer&, BitboardContainer&);

		void getArticulationNodes(PieceNode*, int&);
		void checkArticulationRoot(PieceNode*);

		BitboardContainer getPinnedPieces();
};
