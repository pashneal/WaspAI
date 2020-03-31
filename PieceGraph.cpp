#include <vector>
#include <list>
#include <unordered_map>
#include "PieceGraph.h"



void PieceGraph::insert(BitboardContainer& newBitboard) {
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

void PieceGraph::getArticulationNodes(PieceNode * n, int& counter) {
	//if arrived update node visited
	n -> visited = true;

	n -> visitedNum = counter++;	

	//initially assume that this node is the lowest link
	n -> lowLink = n -> visitedNum;


	for(auto neighbor: n -> neighbors){
		
		if (! (neighbor -> visited) ){

			(*neighbor).parent = n;

			//see if node has a path to lower node
			getArticulationNodes(neighbor, counter);

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
	PieceNode * firstPieceNode = bitboardHashTable.begin() -> second;

	getArticulationNodes(firstPieceNode, counter);
	checkArticulationRoot(firstPieceNode);
	

	BitboardContainer pinned;
	BitboardContainer nextPiece;
	for (auto pinnedPiece: articulationNodes) {
		nextPiece.initialize({{pinnedPiece->boardIndex, pinnedPiece->location}});
		pinned.unionWith(nextPiece);
	}
	return pinned;
}


