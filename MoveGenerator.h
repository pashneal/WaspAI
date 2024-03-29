#pragma once
#include <set>
#include <string>
#include <deque>
#include <vector>
#include <unordered_map>
#include "Bitboard.h"
#include "constants.h"
#include "ProblemNode.h"
#include "PieceGraph.h"

using namespace std;

Bitboard getLegalWalks(Bitboard&, Bitboard&);
bool checkLegalWalk(Bitboard& , Bitboard&, Direction);
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
		void ladybugStep(Bitboard&, Bitboard&, Bitboard&, int);
		void generatePillbugMoves(); 
		void generateMosquitoMoves();
		void generateBeetleMoves();  
		void generateSpiderMoves();  	

	public:

		void generateLadybugMoves(); 
		void generateApproxAntMoves();
		void findAntMoves();
		void generateLegalAntMoves();     

		Bitboard getLegalConnectedComponents(Bitboard);

		bool problemNodesEnabled = false;
		bool approximate = false;
		bool extraInfoOn = false;
		IntermediateGraph graph;

		Bitboard piecesExceptCurrent;
		ProblemNodeContainer *problemNodes;
		Bitboard * allPieces;
		//store locations that affect where a piece can move
		Bitboard extraInfo;

		const int NUM_SPIDER_MOVES = 3;

		MoveGenerator(){};
		MoveGenerator(Bitboard *);

		Bitboard getMoves();

		void setGeneratingName(PieceName *);
		void setGeneratingPieceBoard(Bitboard *);
		void setUpperLevelPieces(Bitboard *);
		void setPieceStacks(unordered_map <int , deque <pair < PieceColor , PieceName >>> * );
		Bitboard getPillbugSwapSpaces();

		Bitboard getInaccessibleNodes(Bitboard);
		Bitboard getInaccessibleNodes(vector <Bitboard> *);
		Bitboard getLegalClimbs(Bitboard&);
		Bitboard getLegalClimb(Bitboard&, Direction);
		Bitboard getLegalWalkPerimeter(Bitboard);

		bool isLegalWalk(Bitboard&, Direction);
};
