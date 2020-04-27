#pragma once
#include <queue>
#include <memory>
#include "MonteCarloNode.h"

#define nodeMap unordered_map<nodePtr, GameState>

int numCores = 1;
int MonteCarloSimulations = 100;
int MonteCarloSimulationsCutoff = 500;
double explorationFactor = 1.41;
double heuristicFactor = .5;

// don't bother attempting to learn from nodes unless 
// its parent has been visited this proportion of times
double minLearningFraction = .1;
// don't bother attempting to learn from nodes unless it has
// a depth <= to this 
int maxLearningDepth = 10;
class MonteCarloTree {
		nodePtr root;
		Heuristic& currentHeuristic;

	public:
		MonteCarloTree(MonteCarloNode* r, Heuristic& h)
			:root(nodePtr(r)),currentHeuristic(h){};
		void select();
		nodeMap selectBestLeaves(int, GameState&);
		//TODO: optimize by storing leaves in ordered map
		queue<MoveInfo> traverseToLeaf(nodePtr&, set<nodePtr>);
		bool expand(nodePtr, GameState&); 
		double simulate() const;
		void backPropagate(double); 
		void train();
		double selectionFunction(MoveInfo, nodePtr);
		MoveInfo search(GameState&);
};
