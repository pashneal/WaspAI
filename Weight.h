#include "GameState.h"
class Weight {
		
	public: 
		virtual bool checkUpdate(MoveInfo);
		double multiplier;
		double score = 0;

		Weight (){}
		Weight (double multiplier): multiplier(multiplier){};

		virtual void calculate(GameState&);
		Weight update(MoveInfo lastMove, GameState& newGameState) {
			if (checkUpdate(lastMove) )  {
				Weight w(multiplier);
				w.calculate(newGameState);
				return w;
			} else {
				return *this;
			}
		}

};


class RandomWeight: public Weight {
	public:
		RandomWeight():Weight{0}{};
		void calculate(GameState&) {}
		bool checkUpdate(MoveInfo) {return false;}
};

class MoveCountWeight: public Weight {
		BitboardContainer prevCalculatedMoves;
		BitboardContainer pinned;
		PieceName pieceName;
	public:
		MoveCountWeight(double multiplier, PieceName n):Weight{multiplier},pieceName{n}{};
		void calculate(GameState& newGameState) {
			
		}

		bool checkUpdate(MoveInfo) {
		}
};
