#pragma once
#include <memory>
#include "MonteCarloNode.h"

int numCores = 1;
int MonteCarloSimulations = 0;
double explorationFactor = 1.41;
double heuristicFactor = .5;

// don't bother attempting to learn from nodes unless 
// its parent has been visited this proportion of times
double minLearningFraction = .1;
// don't bother attempting to learn from nodes unless it has
// a depth <= to this 
int maxLearningDepth = 10;
class MonteCarloTree {

		std::shared_ptr<MonteCarloNode> root;
		std::shared_ptr<MonteCarloNode> currentParentNode;
		Heuristic& currentHeuristic;
		double minWeightScores;
		double maxWeightScores;
	

	public:
		MonteCarloTree(MonteCarloNode* r, Heuristic& h)
			:root(shared_ptr<MonteCarloNode>(r)),currentHeuristic(h){};
		void select();
		void expand(); 
		void simulate();
		void backPropagate(); 
		void search();
		double selectionFunction(MoveInfo);


};
