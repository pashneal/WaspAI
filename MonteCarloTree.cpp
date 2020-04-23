#include <algorithm>
#include "MonteCarloTree.h"
void MonteCarloTree::select(){
	double max = -1.0;
	MoveInfo  maxMoveInfo;
	for (auto iter: currentParentNode->children){
		if (iter.second -> initialWeightScores > max) {
			max = iter.second -> initialWeightScores;
			maxMoveInfo = iter.first;
		}
	}
	currentParentNode = currentParentNode->children[maxMoveInfo];
	currentHeuristic.replayMove(maxMoveInfo);
}

bool MonteCarloTree::expand(int pass) {
	//TODO: distribute between cores
	vector <MoveInfo> newStates = currentHeuristic.generateNewGameStates(); 

	if (!newStates.size() && pass) {
		//if no moves available, make empty move
		currentParentNode->createChild(MoveInfo());
		currentParentNode->evaluateAllChildren(currentHeuristic);
		select();
		//attempt to expand again
		return expand(--pass);
	} else {
		for (MoveInfo m: newStates){
			currentParentNode->createChild(m);
		}
		currentParentNode->evaluateAllChildren(currentHeuristic);
	}
	return true;
}

double MonteCarloTree::simulate() const {
	return currentParentNode -> playout(currentHeuristic);	
}

void MonteCarloTree::backPropagate(double result) {
	//
}


//assumes rootGameState is already up-to-date
MoveInfo MonteCarloTree::search(){
		root->numVisited = 1;
		currentHeuristic.reset();
		for (int i = MonteCarloSimulations; i--; ) {
			currentParentNode = root;
			while (currentParentNode->children.size()) {
				//select best child for expansion
				select();
				currentParentNode->numVisited++;
			}
			
			double result = 0;
			//if moves available from a position
			//simulate and get a result
			if (expand(1)) 
				result = simulate();

		
			if (root->children.size() == 1) {
				MoveInfo onlyMove = root -> children.begin() ->first;
				return onlyMove;
			}

			backPropagate(result);
		}
		train();
		double max = -1;
		MoveInfo maxMoveInfo;
		for (auto iter: root->children) {
			if (iter.second->numVisited) {
				double test = iter.second -> playoutScores/ iter.second -> numVisited;
				if (test > max) {
					max = test;
					maxMoveInfo = iter.first;
				}
			}
		}
		return maxMoveInfo;
}

double MonteCarloTree::selectionFunction(MoveInfo m) {
	const auto child = currentParentNode -> children[m];
	double heuristicEstimation = (child->initialWeightScores - minWeightScores) /
								 (maxWeightScores - minWeightScores);
	int numVisited = std::max(child->numVisited, 1);
	double exploration = sqrt(std::log(currentParentNode->numVisited)/numVisited);
	double meanEstimation = child->playoutScores / numVisited;

	return heuristicEstimation*heuristicFactor +
		   meanEstimation *(1-heuristicFactor) +
		   exploration*explorationFactor;
}
