#pragma once
#include <iostream>
#include <unordered_map>
#include <vector>
#include <list>

using namespace std;

enum PieceName{
	GRASSHOPPER=0,
	QUEEN,
	LADYBUG,
	PILLBUG,
	MOSQUITO,
	BEETLE,
	ANT,
	SPIDER
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

Direction rotateClockWise(Direction dir);
Direction rotateCounterClockWise(Direction dir);

const int ROW_SHIFT = 8;
const int COLUMN_SHIFT = 1;
const int BITBOARD_HEIGHT = 8;
const int BITBOARD_WIDTH = 8;
const int BITBOARD_CONTAINER_SIZE  = 16;
const int BITBOARD_CONTAINER_ROWS = 4;
const int BITBOARD_CONTAINER_COLS = 4;
const unsigned long long ODD_ROWS_BITMASK = 0xff00ff00ff00ff00u;

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
