#include "GameState.h"
#include "Heuristic.h"
#include <stack>
#define INF 100000000000;
class AlphaBeta{
		GameState rootGameState;
		Heuristic heuristic;
		stack<MoveInfo> moveHistory;
		double alpha = -INF;
		double beta = INF;
		double minimax(GameState& childGameState, int depth, bool maximizingPlayer, MoveInfo);
	public: 
		AlphaBeta(GameState init, Heuristic & h) :rootGameState(init),heuristic(h){};
		MoveInfo search(int);
};
