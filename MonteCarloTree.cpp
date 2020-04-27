#include <algorithm>
#include "MonteCarloTree.h"

/*
 *
 * Search from the specified MonteCarloTree::root position for best leaf weights
 * maxSelection = maximum number of leaves selected
 * returns map of pointers and gameStates representing best leaf selections 
 *
 */
nodeMap MonteCarloTree::selectBestLeaves(int maxSelection, GameState& initialGameState){
	nodeMap returnMap;
	map<nodePtr, map<double, MoveInfo, std::greater<double>>> candidateNodes;
	set<nodePtr> visited;

	map<double, MoveInfo, std::greater<double>> rootMoves;
	//store root's children as candidate selections
	for (auto child: root->children) {
		rootMoves[selectionFunction(child.first, root)] = child.first;
	}
	candidateNodes[root] = rootMoves;

	for (int i = maxSelection; i--; ){
		GameState newGameState = initialGameState;

		//find best candidate node
		nodePtr bestPointer = nullptr;
		double bestScore = -HUGE_VAL;
		for (auto node: candidateNodes) {
			if (bestScore < node.second.begin()->first) {
				bestScore = node.second.begin()->first;
				bestPointer = node.first;
			}
		}
		if (bestPointer == nullptr) 
			bestPointer = root;
		else
			//erase so you don't select the same pointer twice
			candidateNodes[bestPointer].erase(bestScore);
		
		//select the best leaf from the given position
		queue<MoveInfo> moveHistory = traverseToLeaf(bestPointer, visited);	
		//replay all moves to get to best leaf
		while (moveHistory.size()) {
			newGameState.replayMove(moveHistory.front());
			moveHistory.pop();
		}

		//store the best leaf and game state	
		returnMap[bestPointer] = newGameState;
		visited.insert(bestPointer);

		//create map from sibling nodes if parent is unique
		auto parent = bestPointer->parent;
		if (candidateNodes.find(parent) == candidateNodes.end()) {
			map<double, MoveInfo, std::greater<double>> parentMoves;
			for (auto child: parent->children) {
				if (child.second != bestPointer)
					parentMoves[selectionFunction(child.first, parent)] = child.first;
			}
		}
	}
	return returnMap;
}
/*
 * Traverse down in the tree via best-first selection while possible
 * parent = nodePtr representing the current position in the tree
 * dissallowed = pointers that the traversal algo is not allowed to pass through
 *
 * returns a list of moves that allow a parent to reach it's best leaf
 * mutates parent to become best leaf
 */
queue <MoveInfo> MonteCarloTree::traverseToLeaf(nodePtr& parent, set<nodePtr> disallowed){
	queue<MoveInfo> moveHistory;
	while(parent->children.size()) {
		double bestScore = -1;
		nodePtr bestLeaf = nullptr;
		MoveInfo bestMove;
		for (auto child : parent->children) {
			if (disallowed.find(child.second) == disallowed.end()) continue;
			if (bestScore < child.second->heuristicScore) {
				bestScore = child.second->heuristicScore;
				bestLeaf = child.second;
				bestMove = child.first;
			}
		}
		//if all children are disallowed
		if (bestLeaf == nullptr) return moveHistory;
		moveHistory.push(bestMove);
		parent = bestLeaf;
	}
	return moveHistory;
}

//Expands a node by looking at all its moves and
//returns the best one
MoveInfo MonteCarloTree::expand(nodePtr leafPtr, GameState& leafGameState) {
	vector<MoveInfo> moves = leafGameState.generateAllMoves();
	if (moves.size()) 
		for (MoveInfo m: moves) 
			leafPtr->createChild(m);
	else 
		moves.push_back(MoveInfo());
	leafPtr->evaluateAllChildren(currentHeuristic, leafGameState);
	return traverseToLeaf(leafPtr, {}).front();
}

double simulate(GameState gameState){
	PieceColor initialTurnColor = gameState.turnColor;
	gameState.playout(MonteCarloSimulationsCutoff);
	if (gameState.checkDraw())
		return .5;
	else if (gameState.checkVictory() != PieceColor::NONE) 
		return 1*(gameState.checkVictory() == initialTurnColor);
	return gameState.approximateEndResult();
};
void MonteCarloTree::backPropagate(double result) {
	//
}

//assumes rootGameState is already up-to-date
MoveInfo MonteCarloTree::search(GameState& initialGameState){
	//reset the tree
	root->clearChildren();		
	root.reset();
}

// math formula for calculating the score of a move 
// tweaked to include a weights guided by heurisitics
double MonteCarloTree::selectionFunction(MoveInfo m, nodePtr currentParent) {
	const auto child = currentParent-> children[m];
	double heuristicEstimation = (child->heuristicScore - 
								  currentParent->minChildScore) /
								 (currentParent->maxChildScore - 
								  currentParent->minChildScore);
	int numVisited = std::max(child->numVisited, 1);
	double exploration = sqrt(std::log(currentParent->numVisited)/numVisited);
	double meanEstimation = child->playoutScore / numVisited;

	return heuristicEstimation*heuristicFactor +
		   meanEstimation *(1-heuristicFactor) +
		   exploration*explorationFactor;
}
