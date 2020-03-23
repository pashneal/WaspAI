#pragma once
#include <set>
#include <string>
#include <vector>
#include <unordered_map>
#include "Bitboard.h"
#include "Hive.h"
#include "constants.h"

using namespace std;

class MoveGenerator {
	
		PieceName * generatingPieceName;

		//TODO: Optimize by checking inaccesible points only after it has been
		//decided on by game engine
		//TODO: make these read-only pointers
		BitboardContainer * allPieces;
		vector <BitboardContainer> * gatesSplit;
		BitboardContainer * generatingPieceBoard;
		BitboardContainer * gatesCombined;


		BitboardContainer moves;
		BitboardContainer perimeter;
	
		bool pieceIsAtopHive;

		void generateMoves();

		void generateGrasshopperMoves();
		void generateQueenMoves();   
		void generateLadybugMoves(); 
		void generatePillbugMoves(); 
		void generateMosquitoMoves();
		void generateBeetleMoves();  
		void generateAntMoves();     
		void generateSpiderMoves();  	
	public:

		vector <Direction> gameDirections = { Direction::NE,
										      Direction::E,
											  Direction::SE,
											  Direction::SW,
											  Direction::N,
											  Direction::S };
		const int NUM_SPIDER_MOVES = 3;

		MoveGenerator() {};

		BitboardContainer getMoves();


		void setGeneratingName(PieceName *);
		void setAllPiecesBoard(BitboardContainer *);		
		void setGatesSplit(vector <BitboardContainer> *);
		void setGeneratingPieceBoard(BitboardContainer *, bool);
		void setGatesCombined(BitboardContainer);
		
		BitboardContainer generatePillbugSwap();

		BitboardContainer getInaccessibleNodes(BitboardContainer);
		BitboardContainer getInaccessibleNodes(vector <BitboardContainer> *);
};
