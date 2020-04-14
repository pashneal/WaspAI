#pragma once
#include <iostream>
#include <unordered_map>
#include <vector>
#include <list>
#include <map>

using namespace std;

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
	S = -2
};


enum PieceColor {
	WHITE = 0,
	BLACK = 1,
	NONE = 2
};

extern int dxdy[6][2];  
extern unordered_map <string, int> pieceNum; 
extern unordered_map < string , vector<Direction> > directions;  
extern list <Direction> hexagonalDirections ;
extern unordered_map <Direction, Direction>  oppositeDirection;

Direction rotateClockWise(Direction dir);
Direction rotateCounterClockWise(Direction dir);
extern bool verticalCmp(int , int);
extern bool horizontalCmp(int, int);

unsigned modulo(int, unsigned);
	
extern vector <unordered_map <PieceName, int>> HivePLM; 
const int ROW_SHIFT = 8;
const int COLUMN_SHIFT = 1;
const int BITBOARD_HEIGHT = 8;
const int BITBOARD_WIDTH = 8;
//might need to change this in case you run into weird errors
const int BITBOARD_CONTAINER_SIZE  = 9;
const int BITBOARD_CONTAINER_ROWS = 3;
const int BITBOARD_CONTAINER_COLS = 3;
const unsigned long long ODD_ROWS_BITMASK = 0xff00ff00ff00ff00u;

extern vector <unordered_map<unsigned long long, unsigned long long[5]>> PERIMETER;
extern int PERIMETER_SIZE;
// directions are set out like this
//
// 
//     5    ▄▟▙▄    0 
//      ▁▄▟██████▙▄▁  
//     ██grasshoppe██ 
// 4   ██G1        ██    1
//     ██w         ██
//     ██          ██
//      ▔▀▜██████▛▀▔
//    3     ▀▜▛▀      2
