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
	bool operator==(const MoveInfo& other) const {
		return ( other.oldPieceLocation == oldPieceLocation &&
				 other.newPieceLocation == newPieceLocation && 
				 other.pieceName == pieceName );
	}
	const string toString(string prefix) const {
		auto LSB1 = newPieceLocation.getLeastSignificantBit();
		auto LSB2 = oldPieceLocation.getLeastSignificantBit();
		return prefix + to_string(LSB1.first)+ "\t" + to_string(LSB1.second)+ "\n" +
			   prefix + to_string(LSB2.first)+ "\t" + to_string(LSB2.second)+ "\n";
	}
};

template <>
struct std::hash<MoveInfo> {
	std::size_t operator()(const MoveInfo& m)const {
		auto LSB1 = m.newPieceLocation.getLeastSignificantBit();
		auto LSB2 = m.oldPieceLocation.getLeastSignificantBit();
		return ((__builtin_ctzll(LSB1.second) << 8) + __builtin_ctzll(LSB2.second));
	}
};

class GameState {
  public:
	GameState(){};
	GameState (vector <unordered_map <PieceName, int>> , PieceColor);
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
	set <PieceName> spawnNames;
	vector <pair<BitboardContainer, BitboardContainer>> swappableEmpty;

	list < pair <BitboardContainer , BitboardContainer > > pieceMoves;
	set <PieceName> possibleNames;

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
	void replayMove(MoveInfo);
	void undoMove(MoveInfo);

	PieceColor checkVictory();
	bool checkDraw();
	double approximateEndResult();

	void destroy();
	
	BitboardContainer * getPieces();
	BitboardContainer * getPieces(PieceName);
	BitboardContainer * getPieces(PieceColor);

	void getAllMoves();
	int	 getAllMovesCount();
	vector<MoveInfo> generateAllMoves();

	BitboardContainer getMosquitoMoves(BitboardContainer);
	BitboardContainer getMosquitoPillbug();

	void changeTurnColor();

	bool makeTrueRandomMove();
	bool makePsuedoRandomMove();
	bool attemptSpawn(int);
	bool attemptMove(vector<movesCollection>& , int);

	int moveApproximation(BitboardContainer, PieceName, bool);

	void findPinnedPieces();

	int playout(int);
	void print();

	void operator=(GameState& other);
};

