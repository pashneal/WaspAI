#pragma once
#include "constants.h"
#include "Piece.h"
#include "Bitboard.h"
#include "PieceNode.h"
#include "ProblemNode.h"
#include "MoveGenerator.h"
#include "PieceGraph.h"
#include <stack>
#include <set>
#include <vector>
#include <string>

using namespace std;

struct MoveInfo {
	BitboardContainer oldPieceLocation;
	BitboardContainer newPieceLocation;
	PieceName pieceName;
};

class GameState {

  public:


	GameState();
	~GameState() { destroy();}

	PieceColor turnColor;
	BitboardContainer allPieces;
	BitboardContainer whitePieces;
	BitboardContainer blackPieces;
	BitboardContainer ants;
	BitboardContainer beetles;
	BitboardContainer spiders;
	BitboardContainer ladybugs;
	BitboardContainer queens;
	BitboardContainer mosquitoes;
	BitboardContainer pillbugs;
	BitboardContainer grasshoppers;
	BitboardContainer firstPieces;
	BitboardContainer upperLevelPieces;



	unordered_map < int , stack < pair <PieceColor, PieceName> > > stackHashTable;

	ProblemNodeContainer problemNodeContainer;
	PieceGraph pieceGraph;
	MoveGenerator moveGenerator;


	int turnCounter = 0;


	MoveInfo insertPiece(BitboardContainer&, PieceName);
	MoveInfo movePiece(BitboardContainer&, BitboardContainer&, PieceName name);

	void undoMove(MoveInfo);

	PieceColor checkVictory();
	bool checkDraw();
	void destroy();
	
	BitboardContainer * getPieces();
	BitboardContainer * getPieces(PieceName);
	BitboardContainer * getPieces(PieceColor);

	pair <BitboardContainer, BitboardContainer> getAllMoves();
	
	void print();

	void changeTurnColor();
};

