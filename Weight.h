#pragma once
#include "GameState.h"
#include <algorithm>
class Weight {
	public: 
		GameState * parentGameState;
		
		double multiplier;

		Weight (){}
		Weight (double multiplier): multiplier(multiplier){};

		//initialize to a parent node 
		virtual void initialize(GameState * g){
			parentGameState = g;
		};
		//after intializing to a parent node, 
		//see what things to tweak in order to update heuristics
		virtual double evaluate(MoveInfo){return 0;};
		virtual void correct(double amount){
			multiplier += amount;
		};
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

class PinningPowerWeight : public Weight {
		unordered_map <PieceName , double> piecePowers {
			{GRASSHOPPER, 1.5},  //worth a bit more than spiders
			{QUEEN,		  4.5},  //best thing to pin
			{LADYBUG,	  2},    //more flexible than grasshoppers
			{PILLBUG,	  .75},  //even if pinned, can still use power
			{MOSQUITO,    3},    //worth a little more than ants
			{BEETLE,      3.5},  //almost as good as having queen pinned
			{ANT,         2.5},
			{SPIDER,      1},    //not very useful to pin
		};
		//an inline pin requires twice as many pieces to unpin as
		//does an elbow Pin
		double inlinePinWeight = 1;
		double elbowPinWeight =  .5;


	public:
		
		PinningPowerWeight(unordered_map<PieceName, double> initialMap){
			for (auto iter: initialMap) 
				piecePowers[iter.first] = iter.second;
		}
		
		double evaluate(MoveInfo) override;
		
};
