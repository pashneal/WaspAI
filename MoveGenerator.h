#pragma once
#include <set>
#include <string>
#include <vector>
#include <unordered_map>
#include "Bitboard.h"
#include "constants.h"
#include "ProblemNode.h"

using namespace std;

class MoveGenerator {
	
		PieceName * generatingPieceName;

		//TODO: Optimize by checking inaccesible points only after it has been
		//decided on by game engine
		//TODO: make these read-only pointers
		vector <BitboardContainer> * gatesSplit;
		//TODO: change everything that is generatingPieceBoard to generatingPiece
		BitboardContainer * generatingPieceBoard;


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
		
		BitboardContainer piecesExceptCurrent;

	public:
		ProblemNodeContainer *problemNodes;
		BitboardContainer * allPieces;

		const int NUM_SPIDER_MOVES = 3;

		MoveGenerator(){};
		MoveGenerator(BitboardContainer *, ProblemNodeContainer *);

		BitboardContainer getMoves();

		void setGeneratingName(PieceName *);
		void setGeneratingPieceBoard(BitboardContainer *, bool);
		
		BitboardContainer generatePillbugSwap();

		BitboardContainer getInaccessibleNodes(BitboardContainer);
		BitboardContainer getInaccessibleNodes(vector <BitboardContainer> *);

		bool checkLegalClimb(BitboardContainer&,BitboardContainer&,BitboardContainer&);
};
