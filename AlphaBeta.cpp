#include "AlphaBeta.h"

MoveInfo AlphaBeta::search(int depth){
	moveHistory = {};
	alpha = -INF;
	beta = INF;
	double max = -INF;
	MoveInfo bestMove;

	GameState childGameState = rootGameState;
	for (auto& move : rootGameState.generateAllMoves()){
		childGameState.replayMove(move);
		double result = minimax(childGameState, depth , 1, move);
		if (result >= max){
			max = result;
			bestMove = move;	
		}
		childGameState.undoMove(move);
	}
	return bestMove;
}

double AlphaBeta::minimax(GameState& childGameState, int depth, bool maximizingPlayer, 
							MoveInfo lastMove){
	cout << "===========" << depth << "==========" << endl;
	if (childGameState.checkDraw()){
		return 0;
	}
	if (childGameState.checkVictory() != PieceColor::NONE) {
		if (maximizingPlayer) return INF;
		return -INF;
	}
	if (depth == 0){
		double value = 0;
		for (auto i : heuristic.evaluate(lastMove)) value += i;
		return value;
	}
	if (maximizingPlayer){
		double value = -INF;
		for (auto move : childGameState.generateAllMoves()){
			//TODO: if about to recalulate move
			if (depth == 1) heuristic.setGameState(childGameState);
			childGameState.replayMove(move);
			value = std::max(value, minimax(childGameState, depth - 1, !maximizingPlayer, move));
			alpha = max(alpha, value);
			
			if (alpha >= beta) break;
			childGameState.undoMove(move);
		}
		return value;
	} else {
		double value = INF;
		for (auto move : childGameState.generateAllMoves()){
			//TODO: if about to recalulate move
			if (depth == 1) heuristic.setGameState(childGameState);
			childGameState.replayMove(move);
			value = std::min(value, 
							minimax(childGameState, depth - 1, !maximizingPlayer, move));
			beta = min(beta, value);
			if (alpha >= beta) break;
			cout << move.toString(" MOVE ") << endl;
			childGameState.undoMove(move);
		}
		return value;
	}

}
