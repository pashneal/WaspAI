#include <vector>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include "PieceGraph.h"


void PieceGraph::insert(Bitboard& newBitboard) {
	if (!(allPieces.containsAny(newBitboard))) {
		PieceNode * newPieceNode = new PieceNode();
		Bitboard perimeter = newBitboard.getPerimeter();

		perimeter.intersectionWith(allPieces);

		list <PieceNode*> neighbors;
		for (auto& bitboard: perimeter.splitIntoBitboards()){
			neighbors.push_front(bitboardHashTable.at(bitboard.hash()));
		}

		newPieceNode -> insert(neighbors, newBitboard);
		bitboardHashTable[newBitboard.hash()] = newPieceNode;

		allPieces.unionWith(newBitboard);
	}
}

void PieceGraph::remove(Bitboard& oldBitboard) {
	if (bitboardHashTable.find(oldBitboard.hash() ) == bitboardHashTable.end()) {
		cout << "attempting to remove piece that does exist in piece graph" << endl;
		oldBitboard.print();
		throw 15;
	}
	PieceNode * p = bitboardHashTable[oldBitboard.hash()];
	p -> remove();
	delete p;
	bitboardHashTable.erase(oldBitboard.hash());
	allPieces.notIntersectionWith(oldBitboard);
	//TODO: xor is faster but not as safe?
}

void PieceGraph::reposition(Bitboard& oldBitboard, Bitboard& newBitboard) {
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
Bitboard PieceGraph::getPinnedPieces(Bitboard root) {
	articulationNodes.clear();
	int counter  = 0;
	if (bitboardHashTable.size() == 0) return Bitboard();
	PieceNode * firstPieceNode;
	if (root == Bitboard()){
		firstPieceNode = bitboardHashTable.begin() -> second;
	} else {
		firstPieceNode = bitboardHashTable.at(root.hash());
	}

	unordered_set<PieceNode*> visited;
	getArticulationNodes(firstPieceNode, counter, visited);
	checkArticulationRoot(firstPieceNode);
	

	Bitboard pinned;
	Bitboard nextPiece;
	for (auto pinnedPiece: articulationNodes) {
		nextPiece.initialize({{pinnedPiece->boardIndex, pinnedPiece->location}});
		pinned.unionWith(nextPiece);
	}
	return pinned;
}


void PieceGraph::destroy() {
	for (auto element : bitboardHashTable ) {
		delete element.second;
	}
	bitboardHashTable.clear();
	articulationNodes.clear();
	allPieces.clear();
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


bool PieceGraph::checkBiDirectional(Bitboard a, Bitboard b) {
	if (bitboardHashTable.find(a.hash()) == bitboardHashTable.end() ||
		bitboardHashTable.find(b.hash()) == bitboardHashTable.end())
	{
		cout << "node does not exist in hash table" << endl;
		throw 2;
	}

	PieceNode * ap = bitboardHashTable[a.hash()];
	PieceNode * bp = bitboardHashTable[b.hash()];

	bool forward = false;
	bool backward = false;
	for (PieceNode * next : ap ->neighbors) {
		forward |= next == bp;
	}
	for (PieceNode * back : bp ->neighbors) {
		backward |= back == ap;
	}

	return forward && backward;
}

void MoveGraph::getArticulationNodes(Bitboard root) {
	articulationLocations.clear();
	PieceGraph::getPinnedPieces(root);

	for (auto node: articulationNodes){
		Bitboard newNode({{node->boardIndex, node->location}});
		articulationLocations.unionWith(newNode);
	}
}

void MoveGraph::destroy(){
	PieceGraph::destroy();
	articulationLocations.clear();
}

Bitboard MoveGraph::getMoves(){
	return allPieces;
}
