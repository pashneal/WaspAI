#pragma once
#include <vector>
#include <list>
#include "PieceNode.h"
#include <unordered_map>
#include <unordered_set>
using namespace std;

class PieceGraph {
	protected:
		//the pieceGraph assumes that the graph is connected
		unordered_set <PieceNode*> articulationNodes{};

		//assumes that this is updated before every insert or remove operation
		Bitboard allPieces;

		unordered_map <int, PieceNode*> bitboardHashTable{};
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

		Bitboard getPinnedPieces(Bitboard = Bitboard());

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

class MoveGraph : public PieceGraph{
	public:
		Bitboard articulationLocations;

		~MoveGraph(){destroy();};
		void getArticulationNodes(Bitboard);
		void destroy();
		Bitboard getMoves();
};

class IntermediateGraph {
		unordered_map<int, boost::container::flat_set<int>> nodes;
		vector<Bitboard> numHashTable;
		Bitboard allNodes;
	public:
		IntermediateGraph(){
			numHashTable.resize(3);
			std::fill(numHashTable.begin(), numHashTable.end(), Bitboard());
		}
		Bitboard root;
		void assignNode(Bitboard& node, int num){
			if (nodes.find(node.hash()) == nodes.end()){
				nodes[node.hash()] = {};
				allNodes.unionWith(node);
			}
			nodes[node.hash()].insert(num);
			numHashTable[num].unionWith(node);
		}
		vector<int> find(Bitboard& test){
			vector<int>v;
			if (test.count() <= 1) {
				if (nodes.find(test.hash()) == nodes.end()) 
					return v;
				for (auto i: nodes[test.hash()]) v.push_back(i);
			} else {
				for (int i = 0; i < 3; i++) 
					if (numHashTable[i].containsAny(test)) 
						v.push_back(i);
			}
			return v;
		}
		void clear(){
			nodes.clear();
			allNodes.clear();
			std::fill(numHashTable.begin(), numHashTable.end(), Bitboard());
		}
		void remove(Bitboard removingNodes) {
			for (auto& node: removingNodes.splitIntoBitboards()) {
				for (int i = 0 ; i < 3 ; i++) {
					numHashTable[i].notIntersectionWith(node);
				}
				nodes.erase(node.hash());
			}
		}
		Bitboard getNodes() {
			return allNodes;
		}
		Bitboard getEndNodes() {
			return numHashTable[2];
		}
		Bitboard getIntermediateNodes(){
			Bitboard ret(numHashTable[1]);
			ret.unionWith(numHashTable[0]);
			return ret;
		}
};
