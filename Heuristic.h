#pragma once
#include <stack>
#include "GameState.h"
#include "Weight.h"
class Heuristic {
		std::stack<MoveInfo> moveInfoTraversal;
		GameState * rootGameState;
	public:
		static int NUMWEIGHTS;
		vector <Weight> weights;
		vector <double> evaluations;

		Heuristic (GameState * rootGameState, Complexity):rootGameState(rootGameState) {};
		void loadWeights(string);
		void saveWeights(string);
		void replayMove(MoveInfo);
		void undoMove();
		void evaluate();
};
