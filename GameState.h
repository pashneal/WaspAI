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

#define movesCollection vector<pair<BitboardContainer,int>>

using namespace std;

struct MoveInfo {
	BitboardContainer oldPieceLocation;
	BitboardContainer newPieceLocation;
	BitboardContainer prevImmobile;
	PieceName pieceName = PieceName::LENGTH;
	PieceColor prevTurnColor = PieceColor::NONE;
};

class GameState {
  public:
	GameState( vector <unordered_map <PieceName, int>>, PieceColor);
	GameState(GameState&);
	~GameState() { destroy();}

	PieceColor turnColor;
	int turnCounter = 0;

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
	BitboardContainer upperLevelPieces;
	BitboardContainer immobile;
	BitboardContainer pinned;    

	vector <unordered_map <PieceName, int>> unusedPieces;

	BitboardContainer pieceSpawns;
	list <PieceName> spawnNames;
	vector <pair<BitboardContainer, BitboardContainer>> swappableEmpty;

	list < pair <BitboardContainer , BitboardContainer > > pieceMoves;
	list <PieceName> possibleNames;

	unordered_map < int , stack < pair < PieceColor , PieceName > > > stackHashTable;

	ProblemNodeContainer problemNodeContainer;
	PieceGraph pieceGraph;
	MoveGenerator moveGenerator;

	MoveInfo movePiece(BitboardContainer&, BitboardContainer&, PieceName);

	void fastMovePiece(BitboardContainer&, BitboardContainer&, PieceName);
	void fastInsertPiece(BitboardContainer&, PieceName);
	void fastRemovePiece(BitboardContainer&, PieceName);
	void fastSpawnPiece(BitboardContainer&, PieceName);

	int countSwaps(BitboardContainer&);
	int countTotalUnusedPieces();

	void randomSpawnPiece(BitboardContainer&);
	void randomSwapPiece(BitboardContainer, BitboardContainer);
	void randomMovePiece(BitboardContainer&, BitboardContainer&, PieceName name);

	BitboardContainer getAllSpawnSpaces();
	pair <BitboardContainer, BitboardContainer> getSwapSpaces(BitboardContainer);

	PieceName findTopPieceName(BitboardContainer);
	PieceColor findTopPieceColor(BitboardContainer);
	void undoMove(MoveInfo);

	PieceColor checkVictory();
	bool checkDraw();
	void destroy();
	
	BitboardContainer * getPieces();
	BitboardContainer * getPieces(PieceName);
	BitboardContainer * getPieces(PieceColor);

	void getAllMoves();
	BitboardContainer getMosquitoMoves(BitboardContainer);
	BitboardContainer getMosquitoPillbug();

	void changeTurnColor();

	bool makePsuedoRandomMove();
	bool attemptSpawn(int);
	bool attemptMove(vector<movesCollection>& , int);

	int moveApproximation(BitboardContainer, PieceName, bool);

	void findPinnedPieces();


	void playout(int);
	void print();

};

