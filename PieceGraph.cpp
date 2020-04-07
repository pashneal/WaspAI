#include <vector>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include "PieceGraph.h"



void PieceGraph::insert(BitboardContainer& newBitboard) {

	if (!(allPieces.containsAny(newBitboard))) {

		PieceNode * newPieceNode = new PieceNode();
		BitboardContainer perimeter = newBitboard.getPerimeter();

		perimeter.intersectionWith(allPieces);

		list <PieceNode*> neighbors;
		for (auto bitboard: perimeter.splitIntoBitboardContainers()){
			neighbors.push_front(bitboardHashTable[bitboard.hash()]);
		}

		newPieceNode -> insert(neighbors, newBitboard);
		bitboardHashTable[newBitboard.hash()] = newPieceNode;

		allPieces.unionWith(newBitboard);
	}

}

void PieceGraph::remove(BitboardContainer& oldBitboard) {
	PieceNode * p = bitboardHashTable[oldBitboard.hash()];
	p -> remove();
	delete p;
	bitboardHashTable.erase(oldBitboard.hash());
	allPieces.notIntersectionWith(oldBitboard);
	//TODO: xor is faster but not as safe?
}

void PieceGraph::reposition(BitboardContainer& oldBitboard, BitboardContainer& newBitboard) {
	remove(oldBitboard);
	insert(newBitboard);
}

void PieceGraph::getArticulationNodes(PieceNode * n, int& counter, 
									 unordered_set<PieceNode*>& visited) {

	visited.insert(n);

	n -> visitedNum = counter++;	

	//initially assume that this node is the lowest link
	n -> lowLink = n -> visitedNum;


	for(auto neighbor: n -> neighbors){
	
		//if not yet visited
		if (visited.find(neighbor) == visited.end() ){

			(*neighbor).parent = n;

			//see if node has a path to lower node
			getArticulationNodes(neighbor, counter, visited);

			//if neighbor points to a lower node
			//update lowest Link
			n -> lowLink = (n -> lowLink < neighbor -> lowLink) ? 
				n -> lowLink : neighbor -> lowLink;

			
			//if neighbor cannot reach a lower node
			//that means this node is not a leaf in a DFS tree
			if ( neighbor -> lowLink >=  n -> visitedNum && n )
				articulationNodes.insert(n);
		}

		//if the node is a back edge
		else if (!(n -> parent == neighbor)) {

			// set lowest link to min(lowlink, neighbor.num)
			n -> lowLink = (n -> lowLink < (*neighbor).visitedNum) ?
						n -> lowLink : (*neighbor).visitedNum;
		}
	}
}

void PieceGraph::checkArticulationRoot(PieceNode * root) {
	int children = 0;
	for (auto potentialChild : root->neighbors){
		if (potentialChild -> parent == root) {
			children++;
			if (children > 1) {articulationNodes.insert(root); return;}
		}
	}
	articulationNodes.erase(root);
}

//TODO: optimize (recalculate O(k) for inserted leaves)
BitboardContainer PieceGraph::getPinnedPieces() {
	articulationNodes.clear();
	int counter  = 0;
	if (bitboardHashTable.size() == 0) return BitboardContainer();
	PieceNode * firstPieceNode = bitboardHashTable.begin() -> second;

	unordered_set<PieceNode*> visited;
	getArticulationNodes(firstPieceNode, counter, visited);
	checkArticulationRoot(firstPieceNode);
	

	BitboardContainer pinned;
	BitboardContainer nextPiece;
	for (auto pinnedPiece: articulationNodes) {
		nextPiece.initialize({{pinnedPiece->boardIndex, pinnedPiece->location}});
		pinned.unionWith(nextPiece);
	}
	return pinned;
}


void PieceGraph::destroy() {
	articulationNodes.clear();
	for (auto element : bitboardHashTable ) {
		PieceNode * ptr = element.second;
			ptr->neighbors.clear();
			ptr->parent = nullptr;
			delete ptr;
	}
	bitboardHashTable.clear();
}

void PieceGraph::DFS( PieceNode * root,
					 unordered_set<PieceNode*>& visited) {

	visited.insert(root);

	for (auto neighbor : root -> neighbors) {
		
		if (visited.find(neighbor) != visited.end() ) continue;
		DFS(neighbor, visited);
	}
}

unordered_set <PieceNode*> PieceGraph::DFS() {
	unordered_set <PieceNode*> visited;
	if (bitboardHashTable.size() != 0)
		DFS(bitboardHashTable.begin() -> second, visited);
	return visited;
}

