#include "constants.h"
#include "Heuristic.h"
class Arena {
		bool playerOneCPU = false;
		bool playerTwoCPU = false;
		vector<MoveInfo> moveHistory;
		vector<string> moveHistoryNotation;
		Heuristic heuristics[2];

		//stores the identity for stacked pieces 
		unordered_map< int, vector<tuple<PieceName, PieceColor, string>>> pieceOrders;
	public:
		GameState currentGameState;
		vector <unordered_map<PieceName,int>> maxUnusedPieces;
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
