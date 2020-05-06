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
		Heuristic currentHeuristic;

	public:
		MonteCarloTree(){};
		MonteCarloTree(Heuristic& h)
			:currentHeuristic(h){};

		void initializeTo(MonteCarloTree& MCT) {
			currentHeuristic = MCT.currentHeuristic;
		}
		
		//toggles the training procedure in search()
		bool trainingMode = false;
		void train(nodePtr, set<nodePtr>&, vector<double>& );
		
		//selection methods
		queue<MoveInfo> selectBestLeaf(nodePtr&);
		double selectionFunction(MoveInfo, nodePtr); 
		
		//Monte Carlo Tree Search Methods
		MoveInfo expand(nodePtr, GameState); 
		double simulate(GameState);
		void backPropagate(nodePtr, double); 
		MoveInfo search(nodePtr, GameState, int);
		MoveInfo multiSearch(GameState&, int);


};
