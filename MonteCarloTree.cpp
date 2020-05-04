#include <algorithm>
#include "MonteCarloTree.h"
// don't bother attempting to learn from nodes unless 
// its parent has been visited this proportion of times
double minLearningFraction = .1;

int intRand(const int & min, const int & max) {
    static thread_local std::mt19937 generator;
    std::uniform_int_distribution<int> distribution(min,max);
    return distribution(generator);
}
/*
 *
 * Search from the specified MonteCarloTree::root position for best leaf weights
 * maxSelection = maximum number of leaves selected
 * returns map of pointers and gameStates representing best leaf selections 
 *
 */
nodeMap MonteCarloTree::selectBestLeaves(int maxSelection, GameState& initialGameState){
	nodeMap returnMap;
	//maps nodePtr -> bestScore, bestMove for all children at nodePtr
	map<nodePtr, map<double, MoveInfo, std::greater<double>>> candidateNodes;
	set<nodePtr> visited;

	map<double, MoveInfo, std::greater<double>> rootMoves;
	//store root's children as candidate selections
	for (auto child: root->children) {
		//child.first = nodePtr to child
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
		else {
			//erase so you don't select the same pointer twice
			candidateNodes[bestPointer].erase(bestScore);
		}
		
		//select the best leaf from the given position
		queue<MoveInfo> moveHistory = traverseToLeaf(bestPointer, visited);	

		//if we could not find best leaf cancel this thread
		if (bestPointer == nullptr) continue;

		//replay all moves to update gameState to leaf
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
 * if a leaf could not be reached, parent = nullptr
 */
queue <MoveInfo> MonteCarloTree::traverseToLeaf(nodePtr& parent, set<nodePtr> disallowed){
	queue<MoveInfo> moveHistory; 
	while(parent->children.size()) {
		double bestScore = -1;
		nodePtr bestLeaf = nullptr;
		MoveInfo bestMove;
		for (auto child : parent->children) {
			if (disallowed.find(child.second) != disallowed.end()) continue;
			if (bestScore < child.second->heuristicScore) {
				bestScore = child.second->heuristicScore;
				bestLeaf = child.second;
				bestMove = child.first;
			}
		}
		//if all children are disallowed
		if (bestLeaf == nullptr) {
			parent = nullptr;
			return moveHistory;
		}
		moveHistory.push(bestMove);
		parent = bestLeaf;
	}
	return moveHistory;
}

//Expands a node by looking at all its moves and
//mutates input MoveInfo to the best move found
void MonteCarloTree::expand(nodePtr leafPtr, GameState leafGameState, MoveInfo& bestMove) {
	vector<MoveInfo> moves = leafGameState.generateAllMoves();

	//if moves were found expand the node
	if (moves.size()) 
		for (MoveInfo m: moves) 
			leafPtr->createChild(m);
	else 
		moves.push_back(MoveInfo());

	leafPtr->evaluateAllChildren(currentHeuristic, leafGameState);
	bestMove = traverseToLeaf(leafPtr, {}).front();
}

//Plays out games until a match resolution or a set limit of moves (whichever is first)
//If the game is decisive, update with 0, 1, or .5 for loss, win or draw
//If the game is not decisive, approximate with values [0-1)
void MonteCarloTree::simulate(GameState gameState, double& result){
	PieceColor initialTurnColor = gameState.turnColor;

	if (!gameState.checkDraw() && gameState.checkVictory() == PieceColor::NONE)
		gameState.playout(MonteCarloSimulationsCutoff);

	if (gameState.checkDraw()){
		result = .5;
		return;
	}
	else if (gameState.checkVictory() != PieceColor::NONE) {
		result =  1*(gameState.checkVictory() == initialTurnColor);
		return;
	}
	result = gameState.approximateEndResult();
};

//Goes up the tree and updates the node augmentations
//Assumes leafPtr is pointing to the leaf where simulate() was called
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

//Given a position, apply MCTS to attempt to find the best move and return it
//If no moves exist from this position, return empty move
//supports multithreading based on numCores variable
//supports training mode based on trainingMode variable
MoveInfo MonteCarloTree::search(GameState& initialGameState){
	//delete the old tree
	if (root != nullptr){
		root->clearChildren();		
		delete root;
	}

	//create a new tree
	root = new MonteCarloNode();
	MoveInfo empty;
	expand(root, initialGameState, empty);

	//if only one move allowed, return it
	if (root -> children.size() == 1) return root->children.begin()->first;
	int numTrials = MonteCarloSimulations;

	while (numTrials>0) {
			
		nodeMap leafNodes = selectBestLeaves(numCores, initialGameState);
		vector<MoveInfo> bestMoves;

		//use multithreading to expand nodes
		for (auto leafNode: leafNodes)
			bestMoves.push_back(MoveInfo());
		vector<std::thread> threads;
		int i = 0;
		for (auto leafNode: leafNodes) {
			nodePtr leaf = leafNode.first;
			GameState g = leafNode.second;
			threads.push_back(
					std::thread(&MonteCarloTree::expand, this, leaf, g, std::ref(bestMoves[i])));
			i++;
		}

		for (int i = threads.size();i--;)
			threads[i].join();

		//use multithreading to perform game simulations
		vector<double> results(leafNodes.size());
		threads.clear();
		i = 0;
		nodeMap newLeafNodes;
		for (auto leafNode: leafNodes){
			//advance the leafNode by playing best move
			GameState g = leafNode.second;
			g.replayMove(bestMoves[i]);
			nodePtr n = leafNode.first;
			n = n->children[bestMoves[i]];

			//assign new gameState
			newLeafNodes[n] = g;

			threads.push_back(
					std::thread(&MonteCarloTree::simulate, this, g, std::ref(results[i])));
			i++;
		}
			
		for (int i = threads.size();i--;)
			threads[i].join();

		//backPropagate for each result
		i = 0;
		for (auto& leafNode: newLeafNodes) {
			backPropagate(leafNode.first,results[i]);
			i++;
		}

		numTrials -= leafNodes.size();
	}

	//train the heuristic model
	if (trainingMode) {
		vector <double> corrections(0, currentHeuristic.NUMWEIGHTS);
		set<nodePtr> visited;
		vector<std::thread> threads;
		for (int i = numCores;i--;)
			threads.push_back(std::thread(&MonteCarloTree::train, this, root,
							  std::ref(visited), std::ref(corrections)));
		for (int i = numCores;i--;)
			threads[i].join();
		currentHeuristic.train(corrections);
	}

	//choose and return best move
	double max = -1;
	MoveInfo bestMove;
	for (auto child: root->children) {
		int denominator = std::max(child.second->numVisited, 1);
		if (child.second->playoutScore/ denominator > max) {
			max = child.second->playoutScore/denominator;
			bestMove = child.first;
		}
	}
	return bestMove;
}

//calculate the corrections needed for every weight in each node
//past a certain depth, corrections are negligible so ignore those nodes
void MonteCarloTree::train(nodePtr node, set<nodePtr>& visited, vector<double>&corrections){
	//if possible children already visited
	mtx.lock();
	if (visited.find(node) != visited.end() ) return;
	mtx.unlock();

	if (node->numVisited / MonteCarloSimulations >= minLearningFraction)  {
		vector<nodePtr> shuffled;
		for (auto keyValue: node->children) shuffled.push_back(keyValue.second);

		//Randomly order the traversal of children to minimize collision with 
		//other threads
		static thread_local std::mt19937 generator(rd());

		std::shuffle(shuffled.begin(), shuffled.end(), generator);

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
				int denominator = std::max(child.second->numVisited, 1);
				max = std::max(child.second->playoutScore / denominator,  max);

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
// tweaked to include weights guided by heuristics
double MonteCarloTree::selectionFunction(MoveInfo m, nodePtr currentParent) {
	const auto child = currentParent-> children[m];
	const double maxChildScore = currentParent->maxChildScore;
	const double minChildScore = currentParent->minChildScore;

	double denominator = ((maxChildScore - minChildScore) < .0001 ) ?
			1 : maxChildScore - minChildScore;
	double heuristicEstimation = (child->heuristicScore - minChildScore) / denominator;

	int childNumVisited = std::max(child->numVisited, 1);
	int parentNumVisited = std::max(currentParent->numVisited, 1);

	double exploration = sqrt(std::log(parentNumVisited)/childNumVisited);
	double meanEstimation = child->playoutScore / childNumVisited;

	return heuristicEstimation*heuristicFactor +
		   meanEstimation *(1-heuristicFactor) +
		   exploration*explorationFactor;
}
