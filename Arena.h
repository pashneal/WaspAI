#pragma once
#include "constants.h"
#include "MonteCarloTree.h"

class Arena {
		bool playerOneCPU = false;
		bool playerTwoCPU = false;
		MonteCarloTree CPU[2];

		//stores the identity for stacked pieces 
		unordered_map< int, vector<tuple<PieceName, PieceColor, string>>> pieceOrders;
	public:
		vector<MoveInfo> moveHistory;
		vector<string> moveHistoryNotation;
		GameState currentGameState;

		Arena(GameState g):currentGameState(g){};

		void setPlayer(int, MonteCarloTree&);

		string convertToNotation(MoveInfo);
		MoveInfo convertFromNotation(string);
		
		string findTopPieceOrder(Bitboard);
		int countPieces(PieceColor, PieceName);

		void makeMove(string);
		void makeMove(MoveInfo);
		void undo(string);
		void undo(MoveInfo);
		
		bool battle(bool);
};
