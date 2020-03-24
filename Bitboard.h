#pragma once
#include <vector>
#include <unordered_map>
#include <iostream>
#include <set>
#include "constants.h"

using namespace std;
class BitboardContainer {

  public:
	//TODO: figure out shift direction O(n) sort so you can get rid of log(n) ordered set
	set<int> internalBoardCache;

	const int ROW_SHIFT = 8;
	const int COLUMN_SHIFT = 1;
	const int BITBOARD_HEIGHT = 8;
	const int BITBOARD_WIDTH = 8;
	const int BITBOARD_CONTAINER_SIZE  = 16;
	const int BITBOARD_CONTAINER_ROWS = 4;
	const int BITBOARD_CONTAINER_COLS = 4;
	const unsigned long long ODD_ROWS_BITMASK = 0xff00ff00ff00ff00u;

	int  boundingBoxes[16][4];
	unsigned long long int internalBoards[16] = {0};

	//TODO: make internalBoardCache safe so we don't have to initialize
	BitboardContainer() {};
	BitboardContainer(unordered_map<int, unsigned long long>);

	void initialize(unordered_map <int, unsigned long long>);
	void initializeTo(BitboardContainer&);
	void setBoard(int , unsigned long long);

	void convertToHexRepresentation(Direction, int);
	void shiftOrthogonalDirection(Direction, int);
	void shiftDirection(Direction);
	void shiftDirection(Direction, int);

	unsigned long long createLowOverflowMask(Direction, int);
	unsigned long long adjustOverflowMask(Direction, int, bool, unsigned long long);

	void findBoundingBoxes(); 
	void findBoundingBoxes(int);

	void floodFillStep(BitboardContainer&, BitboardContainer&);
	void floodFill(BitboardContainer&);

	void pruneCache();
	void clear();

	void unionWith(BitboardContainer&);
	void intersectionWith(BitboardContainer&);
	void xorWith(BitboardContainer&);
	bool containsAny(BitboardContainer&);

	bool equals(BitboardContainer&);

	unordered_map <int ,unsigned long long> duplicateBoard(vector<Direction>);

	void findAllGates(BitboardContainer&);
	void findGatesContainingPiece(BitboardContainer&, unsigned long long, int);
	void findAllGates();

	BitboardContainer getPerimeter();

	unordered_map <int , vector <unsigned long long> > split();
	vector <BitboardContainer> splitIntoConnectedComponents();
};




