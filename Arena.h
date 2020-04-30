#include "constants.h"
#include "Heuristic.h"
class Arena {
		bool playerOneCPU = false;
		bool playerTwoCPU = false;
		vector<MoveInfo> moveHistory;
		vector<string> moveHistoryNotation;
		Heuristic heuristics[2];

		//stores the pieceOrder for stacked pieces 
		unordered_map< int, stack<string>> pieceOrderStack;
	public:
		GameState currentGameState;

		Arena(GameState g):currentGameState(g){};
		void setPlayer(int, Heuristic&);
		string convertToNotation(MoveInfo);
		MoveInfo convertFromNotation(string);
		string findTopPieceOrder(Bitboard);
		int countPieces(PieceColor, PieceName);


		void makeMove(string);
		void makeMove(MoveInfo);
		void undo(string);
		void undo(MoveInfo);
};
