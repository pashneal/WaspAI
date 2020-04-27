#pragma once
#include <stack>
#include "GameState.h"
#include "Weight.h"
class Heuristic {
		int depth = 0;
	public:
		vector <Weight> weights;
		GameState parentGameState;
		static int NUMWEIGHTS;

		Heuristic (Complexity, set<char>);
		void loadWeights(string);
		void load();
		void saveWeights(string);
		void save();
		void train(vector<double>);
		void setGameState(GameState&);
		vector<double> evaluate(MoveInfo);
};
