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

		
		const int NUM_SPIDER_MOVES = 3;

		MoveGenerator() {};

		unordered_map <int , vector < unsigned long long>> getMoves();


		void setGeneratingName(PieceName *);
		void setAllPiecesBoard(BitboardContainer *);		
		void setGatesSplit(vector <BitboardContainer> *);
		void setGeneratingPieceBoard(BitboardContainer *);
		void setGatesCombined(BitboardContainer);

		BitboardContainer getInaccessibleNodes(BitboardContainer);
		BitboardContainer getInaccessibleNodes(vector <BitboardContainer> *);
};
