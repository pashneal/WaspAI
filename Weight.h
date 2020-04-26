#include "GameState.h"
class Weight {
	protected:	
		GameState parentGameState;
	public: 
		double multiplier;

		Weight (){}
		Weight (double multiplier): multiplier(multiplier){};


		//initialize to a parent node 
		virtual void initialize(GameState&g){parentGameState = g;};
		//after intializing to a parent node, 
		//see what things to tweak in order to update heuristics
		virtual double evaluate(MoveInfo);
};


class RandomWeight: public Weight {
	public:
		RandomWeight():Weight{0}{};
		double calculate(MoveInfo) {return 0;};
};

class SimpleMoveCountWeight: public Weight {
	public:
		SimpleMoveCountWeight(double multiplier)
			:Weight{multiplier}{};

		//uses default initilization function
		//void initialize(GameState){};	
		
		//evaluates the position from the point of view of the last
		//person that moved
		double evaluate(MoveInfo m) {
			bool evaluatingPlayer = parentGameState.turnColor;
			
			parentGameState.replayMove(m);
			int totalMoveCount = 0;
			
			//minimize opponent moves 
			//maximize friendly moves
			for (int i = 0; i < 2; i++ ) {
				parentGameState.changeTurnColor();
				int sign = 1;
				if (parentGameState.turnColor != evaluatingPlayer)
					sign = -1;
				totalMoveCount += parentGameState.getAllMovesCount()*sign;
			}
			
			parentGameState.undoMove(m);

			return totalMoveCount;
		}

};

//HEURISTIC SHOULD CHECK FOR VICTORY OR DRAW FIRST
