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

	

	GameState(list <PieceName> possibleNamesIn) {possibleNames = possibleNamesIn;};
	GameState(GameState&);
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
	BitboardContainer immobile;
	BitboardContainer pinned;
	int turnCounter = 0;

	list < pair <BitboardContainer , BitboardContainer > > pieceMoves;
	list < int > numberMoves;
	list <PieceName> possibleNames;
	int totalPossibleMoves;

	unordered_map < int , stack < pair <PieceColor, PieceName> > > stackHashTable;

	ProblemNodeContainer problemNodeContainer;
	PieceGraph pieceGraph;
	MoveGenerator moveGenerator;

	MoveInfo insertPiece(BitboardContainer&, PieceName&);
	MoveInfo movePiece(BitboardContainer&, BitboardContainer&, PieceName&);

	void fastInsertPiece(BitboardContainer&, PieceName&);
	void fastMovePiece(BitboardContainer&, BitboardContainer&, PieceName&);
	void fastRemovePiece(BitboardContainer&, PieceName&);

	PieceName findPieceName(BitboardContainer);
	void undoMove(MoveInfo);

	PieceColor checkVictory();
	bool checkDraw();
	void destroy();
	
	BitboardContainer * getPieces();
	BitboardContainer * getPieces(PieceName);
	BitboardContainer * getPieces(PieceColor);

	void getAllMoves(list <PieceName>);
	void getMosquitoMoves(BitboardContainer);
	BitboardContainer getAllSpawnSpaces();
	
	void print();

	void changeTurnColor();

	void makePsuedoRandomMove();
	int moveApproximation(BitboardContainer, PieceName);

	void findPinnedPieces();

	pair <BitboardContainer, BitboardContainer> getSwapSpaces(BitboardContainer);

};

