#pragma once
#include <thread>
#include <mutex>
#include <queue>
#include <memory>
#include "constants.h"
#include "MonteCarloNode.h"

#define nodeMap unordered_map<nodePtr, GameState>


//TODO: background search while opponent's turn
class MonteCarloTree {
		nodePtr root;
		Heuristic currentHeuristic;
		std::mutex mtx;

	public:
		MonteCarloTree(){};
		MonteCarloTree(MonteCarloNode* r, Heuristic& h)
			:root(nodePtr(r)),currentHeuristic(h){};
		void initializeTo(MonteCarloTree& MCT) {
			root  = MCT.root;
			currentHeuristic = MCT.currentHeuristic;
		}
		//toggles the training procedure in search()
		bool trainingMode = false;
		void train(nodePtr, set<nodePtr>&, vector<double>&);
		
		//selection methods
		nodeMap selectBestLeaves(int, GameState&);
		queue<MoveInfo> traverseToLeaf(nodePtr&, set<nodePtr>);
		double selectionFunction(MoveInfo, nodePtr); 
		
		//Monte Carlo Tree Search Methods
		void expand(nodePtr, GameState, MoveInfo&); 
		void simulate(GameState, double&);
		void backPropagate(nodePtr, double); 
		MoveInfo search(GameState&);


};
