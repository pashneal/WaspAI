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
 * Traverse down in the tree via best-first selection while possible
 * parent = nodePtr representing the current position in the tree
 * dissallowed = pointers that the traversal algo is not allowed to pass through
 *
 * returns a list of moves that allow a parent to reach it's best leaf
 * mutates parent to become best leaf
 * if a leaf could not be reached, parent = nullptr
 */
queue <MoveInfo> MonteCarloTree::selectBestLeaf(nodePtr& parent){
	queue<MoveInfo> moveHistory; 
	std::mt19937 engine(rd());
	std::uniform_int_distribution<int> distribution(0,1000000000);
	while(parent->children.size()) {
		double bestScore = -1;
		nodePtr bestLeaf = nullptr;
		MoveInfo bestMove;
		for (auto child : parent->children) {
			double candidateScore = selectionFunction(child.first, parent);
			double noise = (distribution(engine) % 10000)/1000000.0;
			if (bestScore < candidateScore + noise) {
				bestScore = candidateScore + noise; 
				bestLeaf = child.second;
				bestMove = child.first;
			}
		}
		moveHistory.push(bestMove);
		parent = bestLeaf;
	}
	return moveHistory;
}

//Expands a node by looking at all its moves and
//mutates input MoveInfo to the best move found
MoveInfo MonteCarloTree::expand(nodePtr leafPtr, GameState leafGameState) {
	//if the game is already resolved at this node, do not expand
	if (leafGameState.checkDraw() || leafGameState.checkVictory() != PieceColor::NONE)
		return MoveInfo();

	vector<MoveInfo> moves = leafGameState.generateAllMoves();
	//if no moves were found, pass turn
	if (!moves.size()) 
		moves.push_back(MoveInfo());

	for (MoveInfo m: moves) 
		leafPtr->createChild(m);

	leafPtr->evaluateAllChildren(currentHeuristic, leafGameState);
	return selectBestLeaf(leafPtr).front();
}

//Plays out games until a match resolution or a set limit of moves (whichever is first)
//If the game is decisive, update with 0, 1, or .5 for loss, win or draw
//If the game is not decisive, approximate with values [0-1)
double MonteCarloTree::simulate(GameState gameState){
	PieceColor initialTurnColor = gameState.turnColor;

	if (!gameState.checkDraw() && gameState.checkVictory() == PieceColor::NONE)
		gameState.playout(MonteCarloSimulationsCutoff);

	if (gameState.checkDraw()){
		return .5;
	}
	else if (gameState.checkVictory() != PieceColor::NONE) {
		return (int)(gameState.checkVictory() != initialTurnColor);
	}
	return gameState.approximateEndResult();
};

//Goes up the tree and updates the node augmentations
//Assumes leafPtr is pointing to the leaf where simulate() was called
void MonteCarloTree::backPropagate(nodePtr leafPtr, double result){
	int maxScore = 1;
	while (leafPtr->parent != nullptr) {
		leafPtr->numVisited++;
		leafPtr->playoutScore += result;
		result = maxScore-result;
		leafPtr  = leafPtr->parent;
	}
	leafPtr->numVisited++;
}

//Given a position, apply MCTS to attempt to find the best move and return it
//If no moves exist from this position, return empty move
MoveInfo MonteCarloTree::search(nodePtr root, GameState initialGameState, int numTrials){
	expand(root, initialGameState);

	//if only one or zero moves available
	if (root -> children.size() < 2) {
		return MoveInfo();
	}
	
	while (numTrials--) {
		auto leafNode = root;
		queue<MoveInfo> moveHistory = selectBestLeaf(leafNode);
		GameState newGameState = initialGameState;
		while (moveHistory.size()) {
			newGameState.replayMove(moveHistory.front());
			moveHistory.pop();
		}

		expand(leafNode, newGameState);
		moveHistory = selectBestLeaf(leafNode);
		while(moveHistory.size()){
			newGameState.replayMove(moveHistory.front());
			moveHistory.pop();
		}
		double result = simulate(newGameState);
		backPropagate(leafNode, result);
	}

	//iterate and select best move from root;
	MoveInfo bestMove;
	double bestScore = -1;
	for (auto child: root->children) {
		int denominator = std::max(1, child.second->numVisited);
		if (child.second->playoutScore/denominator > bestScore) {
			bestScore = child.second->playoutScore/denominator;
			bestMove = child.first;
		}
	}
	//run training procedure if enabled
	return bestMove;
}


//multithreaded search of the game space
MoveInfo MonteCarloTree::multiSearch(GameState& initialGameState, int numThreads){
	vector <nodePtr> roots;
	vector <MonteCarloTree*> MCTs;

	vector <std::thread> threads;
	int dividedWork = std::max(MonteCarloSimulations/numThreads, 1);
	for (int i = 0; i < numThreads; i++) {
		roots.push_back(new MonteCarloNode());
		MCTs.push_back(new MonteCarloTree());
		MCTs[i]->currentHeuristic = 
			Heuristic(currentHeuristic.complexity, currentHeuristic.expansionPieces);
		
		threads.push_back(std::thread(&MonteCarloTree::search, MCTs[i], roots[i],
								initialGameState, dividedWork));
	}

	for (auto& thread: threads){
		thread.join();
	}

	if (trainingMode) {
		for (auto& root: roots){
			set<nodePtr> visited;
			vector<double> corrections(currentHeuristic.weights.size());
			train(root, visited, corrections);
			currentHeuristic.train(corrections);
		}
	}
	unordered_map<MoveInfo, pair<double,double>> bestAverageScores;
	for (auto& root: roots) {
		for (auto child: root->children){

			MoveInfo move = child.first;
			int denominator = std::max(child.second->numVisited, 1);
			auto score = std::make_pair(child.second->playoutScore, denominator);

			if (bestAverageScores.find(move) == bestAverageScores.end())
				bestAverageScores.insert(std::make_pair(move, score));
			else {
				bestAverageScores[move].first += score.first;
				bestAverageScores[move].second += score.second;
			}
		}
	}

	double bestScore = -111;
	MoveInfo bestMove;
	//calculate the best move from given options
	string a;
	for (auto moveIter: bestAverageScores){
		auto fraction = moveIter.second;
		a += to_string(fraction.first) + "/" + to_string((int)fraction.second) + " ";
		if (fraction.first/fraction.second > bestScore){
			bestScore = fraction.first/fraction.second;
			bestMove = moveIter.first;
		}
	}
	cout << a << endl;
	//clean up
	for (auto& root: roots){
		root->clearChildren();
		delete root;
	}
	return bestMove;
}

//calculate the corrections needed for every weight in each node
//past a certain depth, corrections are negligible so ignore those nodes
void MonteCarloTree::train(nodePtr node, set<nodePtr>& visited, vector<double>& corrections){
	if (node->numVisited / MonteCarloSimulations >= minLearningFraction)  {
		vector<nodePtr> children;
		for (auto keyValue: node->children) children.push_back(keyValue.second);

		//perform a DFS on all nodes in the tree and train using results
		for (auto child: children) 
			train(child, visited, corrections);
	}

	//if visited was updated while traversing children
	if (visited.find(node) != visited.end()) 
		return;
	visited.insert(node);

	//if the maxAvgScore is unitilialized, calculate it
	if (node->parent != nullptr)
		if (node->parent->maxAvgScore == -1) {
			double max = -1;
			for (auto child: node->parent->children) {
				int denominator = std::max(child.second->numVisited, 1);
				max = std::max(child.second->playoutScore / denominator,  max);
			}
			node->parent->maxAvgScore = max;
		}

	vector<double> myCorrec = node->train(node->parent->maxChildScore, 
									node->parent->minChildScore,
									node->parent->maxAvgScore);

	for (unsigned i = 0; i < myCorrec.size() ; i++) {
		corrections[i] += myCorrec[i];
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
