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

		//TODO: make these read-only pointers
		BitboardContainer * allPieces;
		vector <BitboardContainer> * allGates;
		BitboardContainer * generatingPieceBoard;

		BitboardContainer moves;
		BitboardContainer perimeter;

		void generateGrasshopperMoves();
		void generateQueenMoves();   
		void generateLadybugMoves(); 
		void generatePillbugMoves(); 
		void generateMosquitoMoves();
		void generateBeetleMoves();  
		void generateAntMoves();     
		void generateSpiderMoves();  
			
	public:

		

		MoveGenerator() {};

		unordered_map <int , vector < unsigned long long>> getMoves();

		void generateMoves();

		void setGeneratingName(PieceName *);
		void setAllPiecesBoard(BitboardContainer *);		
		void setAllGates(vector <BitboardContainer> *);
		void setGeneratingPieceBoard(BitboardContainer *);

		BitboardContainer getInaccessibleNodes();
};
