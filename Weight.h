#include "GameState.h"
class Weight {
	protected:	
		GameState * parentGameState;
	public: 
		Bitboard watchPoints;
		double multiplier;

		Weight (){}
		Weight (double multiplier): multiplier(multiplier){};


		//initialize to a parent node 
		virtual void initialize(GameState * g){parentGameState = g;};
		//after intializing to a parent node, 
		//see what things to tweak in order to update heuristics
		virtual double evaluate(MoveInfo){return 0;};
};


class RandomWeight: public Weight {
	public:
		RandomWeight():Weight{0}{};
		double calculate(MoveInfo) {return 0;};
};


class KillShotCountWeight: public Weight {
		//places in the hive that would trigger a recount
		Bitboard watchPoints;
		Bitboard pinnedWatchPoints;
		Bitboard unpinnedWatchPoints;
		double scores[2];
		int queenCount;
	public:	

		KillShotCountWeight(double multiplier) :Weight(multiplier){};
		void initialize(GameState * g) override;
		double evaluate(MoveInfo) override;
		vector<int> recalculate();
};

class PinnedWeight : public Weight {
	public: 
		PinnedWeight(double multiplier) : Weight(multiplier) {};
		double evaluate(MoveInfo) override;
};

class PieceCountWeight : public Weight{
		int pieceCounts[2];
	public:
		PieceCountWeight(double multiplier) :Weight(multiplier) {};
		void initialize(GameState*) override;
		double evaluate(MoveInfo) override;
};

class MoveCountWeight: public Weight {
		unordered_map<int, Bitboard> oldLocationWatchpoints;
		unordered_map<int, Bitboard> newLocationWatchpoints;
	public: 
		MoveCountWeight(double multiplier) : Weight(multiplier){};
};

class SimpleMoveCountWeight: public Weight {
		PieceName name;
		int moveCounts[2];
		MoveGenerator moveGenerator;
	public:
		SimpleMoveCountWeight(double multiplier, PieceName n) :Weight(multiplier),name(n){};
		void initialize(GameState *) override;
		double evaluate(MoveInfo) override;
		vector<int> recalculate();
};

class AntMoveCountWeight : public MoveCountWeight {
};
class GrasshopperMoveCountWeight : public MoveCountWeight {
};
class LadybugMoveCountWeight: public MoveCountWeight {
};
class MosquitoMoveCountWeight: public MoveCountWeight {
};
