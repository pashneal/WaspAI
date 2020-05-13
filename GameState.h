#pragma once
#include "constants.h"
#include "Bitboard.h"
#include "PieceNode.h"
#include "ProblemNode.h"
#include "MoveGenerator.h"
#include "PieceGraph.h"
#include <deque>
#include <set>
#include <vector>
#include <string>

#define movesCollection vector<pair<Bitboard,int>>

extern Bitboard startSpawnBoard;
using namespace std;

struct MoveInfo {
	Bitboard oldPieceLocation;
	Bitboard newPieceLocation;
	Bitboard prevImmobile;
	PieceName pieceName = PieceName::LENGTH;
	bool operator==(const MoveInfo& other) const {
		return ( other.oldPieceLocation == oldPieceLocation &&
				 other.newPieceLocation == newPieceLocation && 
				 other.pieceName == pieceName );
	}
	const string toString(string prefix) const {
		auto LSB1 = newPieceLocation.getLeastSignificantBit();
		auto LSB2 = oldPieceLocation.getLeastSignificantBit();
		return prefix + to_string(LSB1.first)+ "\t" + to_string(LSB1.second)+ "\n" +
			   prefix + to_string(LSB2.first)+ "\t" + to_string(LSB2.second)+ "\n" +
			   prefix + to_string(pieceName) + "\n";
	}
};
template <>
struct std::hash<MoveInfo> {
       std::size_t operator()(const MoveInfo& m)const {
               auto LSB1 = m.newPieceLocation.getLeastSignificantBit();
               auto LSB2  = m.oldPieceLocation.getLeastSignificantBit();
               return  ((__builtin_ctzll(LSB1.second) << 8)
                               + __builtin_ctzll(LSB2.second));

	   };
};

class GameState {
  public:
	GameState(){};
	GameState (vector <unordered_map <PieceName, int>> , PieceColor);
	GameState(const GameState&);
	~GameState() { destroy();}

	PieceColor turnColor;
	int turnCounter = 0;

	Bitboard allPieces;
	Bitboard whitePieces;
	Bitboard blackPieces;
	Bitboard ants;
	Bitboard beetles;
	Bitboard spiders;
	Bitboard ladybugs;
	Bitboard queens;
	Bitboard mosquitoes;
	Bitboard pillbugs;
	Bitboard grasshoppers;
	Bitboard upperLevelPieces;
	Bitboard immobile;
	Bitboard pinned;    

	vector <unordered_map <PieceName, int>> unusedPieces;

	Bitboard pieceSpawns;
	set <PieceName> spawnNames;
	vector <pair<Bitboard, Bitboard>> swappableEmpty;

	list < pair <Bitboard , Bitboard > > pieceMoves;
	set <PieceName> possibleNames;

	unordered_map < int , deque < pair < PieceColor , PieceName > > > pieceStacks;

	ProblemNodeContainer problemNodeContainer;
	PieceGraph pieceGraph;
	MoveGenerator moveGenerator;

	MoveInfo movePiece(Bitboard&, Bitboard&, PieceName);

	void fastMovePiece(Bitboard&, Bitboard&);
	void fastInsertPiece(Bitboard&, PieceName, PieceColor);
	void fastRemovePiece(Bitboard&);
	void fastSpawnPiece(Bitboard&, PieceName);

	int countSwaps(Bitboard&);
	int countTotalUnusedPieces();

	void randomSpawnPiece(Bitboard&);
	void randomSwapPiece(Bitboard, Bitboard);
	void randomMovePiece(Bitboard&, Bitboard&);

	Bitboard getAllSpawnSpaces();
	pair <Bitboard, Bitboard> getSwapSpaces(Bitboard);

	PieceName findTopPieceName(Bitboard);
	PieceColor findTopPieceColor(Bitboard);
	PieceName findBottomPieceName(Bitboard);
	PieceColor findBottomPieceColor(Bitboard);
	void replayMove(MoveInfo);
	void undoMove(MoveInfo);

	PieceColor checkVictory();
	bool checkDraw();
	double approximateEndResult();

	void destroy();
	
	Bitboard * getPieces();
	Bitboard * getPieces(PieceName);
	Bitboard * getPieces(PieceColor);

	void getAllMoves();
	int	 getAllMovesCount();
	vector<MoveInfo> generateAllMoves();

	Bitboard getMosquitoMoves(Bitboard);
	Bitboard getMosquitoPillbug();

	void changeTurnColor();

	bool makeTrueRandomMove();
	bool makePsuedoRandomMove();
	bool attemptSpawn(int);
	bool attemptMove(vector<movesCollection>& , int);

	int moveApproximation(Bitboard, bool);

	void findPinnedPieces();

	int playout(int);
	void print();

	void operator=(const GameState& other);
};

