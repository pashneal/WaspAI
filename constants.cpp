#include "constants.h"

Direction rotateClockWise(Direction dir) {
	switch (dir) {
		case NE: 
			return Direction::E;
		case E: 
			return Direction::SE;
		case SE: 
			return Direction::SW;
		case SW: 
			return Direction::W;
		case W : 
			return Direction::NW;
		case NW: 
			return Direction::NE;
		default: 
			std::cout << "cannot rotateClockWise nonHexagonalDirection" << std::endl;
			throw 20;
	}
}
Direction rotateCounterClockWise(Direction dir) {

	switch (dir) {
		case NE: 
			return Direction::NW;
		case E: 
			return Direction::NE;
		case SE: 
			return Direction::E;
		case SW: 
			return Direction::SE;
		case W : 
			return Direction::SW;
		case NW: 
			return Direction::W;
		default: 
			std::cout << "cannot rotateCounterClockWise nonHexagonalDirection" << std::endl;
			throw 21;
	}
}

unsigned modulo( int value, unsigned m) {
	int mod = value % (int)m;
	if (value < 0) {
		mod += m;
	}
	return mod;
}

vector < unordered_map <PieceName, int>> HivePLM = 
{
	{	
		{PieceName::GRASSHOPPER, 3},
		{PieceName::QUEEN, 1},
		{PieceName::LADYBUG, 1},
		{PieceName::PILLBUG, 1},
		{PieceName::MOSQUITO, 1},
		{PieceName::BEETLE, 2},
		{PieceName::ANT, 3},
		{PieceName::SPIDER, 2}
	},

	{	
		{PieceName::GRASSHOPPER, 3},
		{PieceName::QUEEN, 1},
		{PieceName::LADYBUG, 1},
		{PieceName::PILLBUG, 1},
		{PieceName::MOSQUITO, 1},
		{PieceName::BEETLE, 2},
		{PieceName::ANT, 3},
		{PieceName::SPIDER, 2}
	}
};

// Takes in two values representing two bitboardLocations
// returns which bitboard is located higher than the other
bool verticalCmp(int a, int b) {
	return a / BITBOARD_CONTAINER_ROWS  > b / BITBOARD_CONTAINER_COLS;
}
