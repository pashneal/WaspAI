#pragma once
#include <random>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <list>
#include <map>

using namespace std;
enum Complexity {
	RANDOM=0,
	SIMPLE=1,
};
enum PieceName{
	GRASSHOPPER=0,
	QUEEN=1,
	LADYBUG=2,
	PILLBUG=3,
	MOSQUITO=4,
	BEETLE=5,
	ANT=6,
	SPIDER=7,
	LENGTH=8,
};

enum Direction{
	NE = 0,
	E = 1,
	SE = 2,
	SW = 3,
	W = 4,
	NW = 5,
	N = -1,
	S = -2,
};


enum PieceColor {
	WHITE = 0,
	BLACK = 1,
	NONE = 2
};

extern int MonteCarloSimulations;
extern int MonteCarloSimulationsCutoff;
extern int dxdy[6][2];  
extern unordered_map <string, int> pieceNum; 
extern unordered_map < string , vector<Direction> > directions;  
extern list <Direction> hexagonalDirections ;
extern unordered_map <Direction, Direction>  oppositeDirection;
extern std::random_device rd;
extern std::mt19937_64 e2;
extern std::uniform_int_distribution<unsigned long long int> dist;
Direction rotateClockWise(Direction dir);
Direction rotateCounterClockWise(Direction dir);

unsigned modulo(int, unsigned);
	
extern vector <unordered_map <PieceName, int>> HivePLM; 
const int ROW_SHIFT = 8;
const int COLUMN_SHIFT = 1;
const int BITBOARD_HEIGHT = 8;
const int BITBOARD_WIDTH = 8;
const int BITBOARD_CONTAINER_SIZE  = 9;
const int BITBOARD_CONTAINER_ROWS = 3;
const int BITBOARD_CONTAINER_COLS = 3;
const unsigned long long ODD_ROWS_BITMASK = 0xff00ff00ff00ff00u;

extern vector <unordered_map<unsigned long long, unsigned long long[5]>> PERIMETER;
extern int PERIMETER_SIZE;
extern unordered_map <unsigned long long, unsigned long long> GATES[64];
