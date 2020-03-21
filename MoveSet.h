#pragma once
#include <set>
#include <string>
#include <vector>
#include <unordered_map>
#include "Bitboard.h"
#include "constants.h"

using namespace std;

class MoveSet {
	
	public:
		PieceName pieceName;
		BitboardContainer allPieces;
		BitboardContainer allGates;
		BitboardContainer moves;
		BitboardContainer perimeter;

		MoveSet() {};
		MoveSet(PieceName, BitboardContainer, BitboardContainer);

		unordered_map <int , vector < unsigned long long>> getMoves();

		void generateMoves();

		void generateGrasshopperMoves();
		void generateQueenMoves();   
		void generateLadybugMoves(); 
		void generatePillbugMoves(); 
		void generateMosquitoMoves();
		void generateBeetleMoves();  
		void generateAntMoves();     
		void generateSpiderMoves();  
			
		
};
