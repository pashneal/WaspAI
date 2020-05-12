#pragma once
#include <set>
#include <string>
#include <deque>
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
		//TODO: change everything that is generatingPieceBoard to generatingPiece
		Bitboard * generatingPieceBoard;
		unordered_map < int, deque < pair < PieceColor , PieceName> > > * pieceStacks; 
		Bitboard * upperLevelPieces;

		Bitboard moves;
		Bitboard perimeter;
	
		void generateMoves();

		void generateGrasshopperMoves();
		void generateQueenMoves();   
		void generateLadybugMoves(); 
		void ladybugStep(Bitboard&, Bitboard&, Bitboard&, int);
		void generatePillbugMoves(); 
		void generateMosquitoMoves();
		void generateBeetleMoves();  
		void generateSpiderMoves();  	
		
		Bitboard piecesExceptCurrent;

	public:

		void generateLegalAntMoves();     
		void generateApproxAntMoves();

		bool problemNodesEnabled = false;
		bool approximate = false;

		ProblemNodeContainer *problemNodes;
		Bitboard * allPieces;
		//store locations that affect where a piece can move
		Bitboard intermediate;

		const int NUM_SPIDER_MOVES = 3;

		MoveGenerator(){};
		MoveGenerator(Bitboard *, ProblemNodeContainer *);

		Bitboard getMoves();

		void setGeneratingName(PieceName *);
		void setGeneratingPieceBoard(Bitboard *);
		void setUpperLevelPieces(Bitboard *);
		void setPieceStacks(unordered_map <int , deque <pair < PieceColor , PieceName >>> * );
		Bitboard getPillbugSwapSpaces();

		Bitboard getInaccessibleNodes(Bitboard);
		Bitboard getInaccessibleNodes(vector <Bitboard> *);
		Bitboard getLegalClimb(Bitboard&, Direction);
		Bitboard getLegalWalkPerimeter(Bitboard);

		bool isLegalWalk(Bitboard&, Direction);
};
