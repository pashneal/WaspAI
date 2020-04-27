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

	//if moves were found expand the node
	if (moves.size()) 
		for (MoveInfo m: moves) 
			leafPtr->createChild(m);
	else 
		moves.push_back(MoveInfo());

	leafPtr->evaluateAllChildren(currentHeuristic, leafGameState);
	return traverseToLeaf(leafPtr, {}).front();
}

//Plays out games until a match resolution or a set limit of moves (whichever is first)
//If the game is decisive, update with 0, 1, or .5
//If the game is not decisive, approximate with values [0-1)
double simulate(GameState gameState){
	PieceColor initialTurnColor = gameState.turnColor;

	if (gameState.checkDraw() || gameState.checkVictory() == PieceColor::NONE)
		gameState.playout(MonteCarloSimulationsCutoff);

	if (gameState.checkDraw())
		return .5;
	else if (gameState.checkVictory() != PieceColor::NONE) 
		return 1*(gameState.checkVictory() == initialTurnColor);
	return gameState.approximateEndResult();
};
//Goes up the tree and updates the node augmentations
//Assumes leafPtr is pointing to lowest node
void MonteCarloTree::backPropagate(nodePtr leafPtr, double result){
	int maxScore = 1;
	while (leafPtr != root) {
		leafPtr->numVisited++;
		leafPtr->playoutScore += result;
		result = maxScore-result;
		leafPtr  = leafPtr->parent;
	}
	root->numVisited++;
}

//assumes rootGameState is already up-to-date
MoveInfo MonteCarloTree::search(GameState& initialGameState){
	//delete the old tree
	root->clearChildren();		
	root.reset();

	//create a new tree
	root = nodePtr(new MonteCarloNode);
	expand(root, initialGameState);
	//if only one move allowed, return it
	if (root -> children.size() == 1) return root->children.begin()->first;
	int numTrials = MonteCarloSimulations;
	while (numTrials) {
	}

	//train the heuristic model
	vector <double> corrections(0, Heuristic::NUMWEIGHTS);
	set<nodePtr> visited;
	vector<std::thread> threads;
	for (int i = numCores;i--;)
		threads.push_back(std::thread(&MonteCarloNode::train, this, root,
						  std::ref(visited), std::ref(corrections)));
	for (int i = numCores;i--;)
		threads[i].join();


	//choose and return best move
	double max = -1;
	MoveInfo bestMove;
	for (auto child: root->children) {
		if (child.second->playoutScore/ child.second->numVisited > max) {
			max = child.second->playoutScore/child.second->numVisited;
			bestMove = child.first;
		}
	}
	return bestMove;
}


void MonteCarloTree::train(nodePtr node, set<nodePtr>& visited, vector<double>&corrections){

	//if possible children already visited
	if (visited.find(node) != visited.end() ) return;

	//traverse the children in a random order to minimize collision
	//with other threads
	if (node->numVisited / MonteCarloSimulations > minLearningFraction)  {

		vector <nodePtr> shuffled;
		for (auto keyValue: node->children) shuffled.push_back(keyValue.second);
		std::random_shuffle(shuffled.begin(), shuffled.end());

		//perform a DFS on all nodes in the tree and train using results
		for (auto child: shuffled) 
			train(child, visited, corrections);
	}

	mtx.lock();
	//if visited was updated while traversing children
	if (visited.find(node) != visited.end()) {
		mtx.unlock();
		return;
	}
	visited.insert(node);
	mtx.unlock();

	//if the maxAvgScore is unitilialized, calculate it
	if (node != root)
		if (node->parent->maxAvgScore == -1) {
			double max = -1;
			for (auto child: node->parent->children) {
				max = std::max(child.second->playoutScore / child.second->numVisited, max);
			}
			//lock threads to prevent race condition
			mtx.lock();
			node->parent->maxAvgScore = max;
			mtx.unlock();
		}

	vector<double> myCorrec = node->train(node->parent->maxChildScore, 
			node->parent->minChildScore,
			node->parent->maxAvgScore);

	//update corrections while locking threads
	for (unsigned i = 0; i < myCorrec.size() ; i++) {
		mtx.lock();
		corrections[i] += myCorrec[i];
		mtx.unlock();
	}
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
