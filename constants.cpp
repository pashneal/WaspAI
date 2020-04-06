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

