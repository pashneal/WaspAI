#include "GameState.h"
#include <algorithm>
class Weight {
	protected:	
		GameState * parentGameState;
		MoveGenerator moveGen;
	public: 
		Bitboard watchPoints;
		unordered_map <int , pair<PieceColor, int>> finalMoveCounts;
		
		double multiplier;

		Weight (){}
		Weight (double multiplier): multiplier(multiplier){};

		//initialize to a parent node 
		virtual void initialize(GameState * g){
			parentGameState = g;
			moveGen.setPieceStacks(&parentGameState->pieceStacks);
			moveGen.setUpperLevelPieces(&parentGameState->upperLevelPieces);
			moveGen.allPieces = &parentGameState->allPieces;
		};
		//after intializing to a parent node, 
		//see what things to tweak in order to update heuristics
		virtual double evaluate(MoveInfo){return 0;};
		virtual int  recalculateMoves(Bitboard);
		double calculateScore() {
			double results = 0;
			for (auto iter: finalMoveCounts){
				auto& pieceColorCount = iter.second;
				if (parentGameState->turnColor == pieceColorCount.first )
					results -= pieceColorCount.second;
				else
					results += pieceColorCount.second;
			}
			results *= multiplier;
			return results;
		}
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
class AntMoveCountWeight : public Weight {
		Bitboard freeAnts;
		Bitboard pinnedAnts;
		Bitboard coveredAnts;
		Bitboard allAnts;
		unordered_map <int ,pair<PieceColor, int> > initialMoveCounts;
		unordered_map <int , MoveGraph> antMoves;
		MoveGenerator moveGen;
		PieceName name = ANT;
	public:
		AntMoveCountWeight(double multiplier) :Weight(multiplier){
			moveGen.setGeneratingName(&name);
		};
		void initialize(GameState *) override;
		double evaluate(MoveInfo) override;
		void correctAssumptions(MoveInfo, Bitboard);
};
class LadybugMoveCountWeight: public Weight {
		Bitboard freeLadybugs;
		Bitboard immobileLadybugs;
		Bitboard upperLevelGates;
		vector<Bitboard> ladybugMoves{Bitboard(), Bitboard()};
		vector<IntermediateGraph> moveGraphs{IntermediateGraph(), IntermediateGraph()};
		MoveGenerator moveGen;
		PieceName name = LADYBUG;
	public:
		LadybugMoveCountWeight(double multiplier) :Weight(multiplier) {
			moveGen.setGeneratingName(&name);
			moveGen.extraInfoOn = true;
		}
		void initialize(GameState *) override;
		double evaluate(MoveInfo) override;
		Bitboard getDependencies(Bitboard, PieceColor);
};
class GrasshopperMoveCountWeight : public Weight {
		Bitboard watchpoints;
		Bitboard freeGrasshoppers;
		Bitboard immobileGrasshoppers;
		unordered_map <int , int> grasshopperMoves;
		unordered_map <int, pair<PieceColor, int>> initialMoveCounts;
	public:
		GrasshopperMoveCountWeight(double multiplier) :Weight(multiplier){ };
		void initialize(GameState *) override;
		double evaluate(MoveInfo) override;
};
class SimpleMoveCountWeight : public Weight {
		PieceName name;
	public: 
		SimpleMoveCountWeight(double multiplier, PieceName n) :Weight(multiplier),name(n){
			moveGen.setGeneratingName(&name);
		};
		double evaluate(MoveInfo) override;
};
class MosquitoMoveCountWeight: public Weight {
		vector<Weight*> weights;
	public:
		MosquitoMoveCountWeight(double multiplier, vector<Weight*> proxies)
		:Weight(multiplier),weights(proxies){}
		double evaluate(MoveInfo) override;
};

