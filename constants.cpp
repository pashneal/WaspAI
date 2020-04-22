#include "constants.h"

unordered_map<Direction, Direction> oppositeDirection = 
{
	{Direction::N, Direction::S},
	{Direction::E, Direction::W},
	{Direction::W, Direction::E},
	{Direction::S, Direction::N},
	{Direction::NW, Direction::SE},
	{Direction::NE, Direction::SW},
	{Direction::SW, Direction::NE},
	{Direction::SE, Direction::NW}
};
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
