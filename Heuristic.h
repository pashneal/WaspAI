#pragma once
#include <stack>
#include "GameState.h"
#include "Weight.h"
class Heuristic {
		int depth = 0;
		GameState * rootGameState;
	public:
		static int NUMWEIGHTS;
		vector <Weight> weights;
		vector <double> evaluations;

		Heuristic (GameState* r, Complexity c, set<char> expansionPieces):rootGameState(r){};
		void loadWeights(string);
		void saveWeights(string);
		void replayMove(MoveInfo);
		void undoMove();
		vector <MoveInfo> generateNewGameStates();
		void evaluate();
		void reset(){depth = 0;};
};
