#pragma once
#include <thread>
#include <mutex>
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

//TODO: background search while opponent's turn
class MonteCarloTree {
		nodePtr root;
		Heuristic& currentHeuristic;
		std::mutex mtx;

	public:
		bool trainingMode = false;
		MonteCarloTree(MonteCarloNode* r, Heuristic& h)
			:root(nodePtr(r)),currentHeuristic(h){};
		nodeMap selectBestLeaves(int, GameState&);
		//TODO: optimize by storing leaves in ordered map
		queue<MoveInfo> traverseToLeaf(nodePtr&, set<nodePtr>);
		void expand(nodePtr, GameState, MoveInfo&); 
		void simulate(GameState, double&);
		void backPropagate(nodePtr, double); 
		void train(nodePtr, set<nodePtr>&, vector<double>&);
		double selectionFunction(MoveInfo, nodePtr); 
		MoveInfo search(GameState&);
};
