#include "constants.h"
#include "Heuristic.h"
class Arena {
		bool playerOneCPU = false;
		bool playerTwoCPU = false;
		Heuristic heuristics[2];
		//stores the first second and third pieces placed into the game
		vector <BitboardContainer> pieceOrder;
		//stores the pieceOrder for stacked pieces 
		//this helps to achieve O(1) time complexity for Arena::move()
		unordered_map< BitboardContainer, stack<int>> pieceOrderStack;
		GameState currentGameState;
	public:
		Arena(GameState& g):currentGameState(g){};
		void setPlayer(int, Heuristic&);
		string convertToNotation(MoveInfo);
		MoveInfo convertFromNotation(string);
		string findTopPieceOrder(BitboardContainer piece);
};
