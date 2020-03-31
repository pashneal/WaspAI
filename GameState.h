#pragma once
#include "constants.h"
#include "Piece.h"
#include "Bitboard.h"
#include "PieceNode.h"
#include <set>
#include <vector>
#include <string>

using namespace std;

class GameState {


  public:

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
	BitboardContainer problemNodes;
	BitboardContainer doors;
	BitboardContainer rings;
	BitboardContainer firstPieces;
	BitboardContainer secondPieces;
	BitboardContainer thirdPieces;


	int turnCounter = 0;

	GameState();

	void insertPiece(BitboardContainer&, PieceName);
	void print();
	void movePiece(char, string, Direction, char, string);
	void movePiece(int, Direction, int);
	bool parseCommand(vector<string>);
};

