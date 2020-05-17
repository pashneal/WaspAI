/*
 * This module represents the various methods and techniques used to implement the concept 
 * of a bitboard in Hive. Bitboards can be found in many popular AI engines and is a useful
 * way of distilling information about a game's state. This is because representing the game at such
 * a low level provides lightning-fast operations to manipulate it.
 *
 * The issue with traditional bitboards is that they are implemented for fixed-size games 
 * where the layout of the board is static and known. For example, see bitboards for chess and
 * checkers.
 *
 * In Hive, it is not possible to bound the bitboard in that way, so techniques were invented
 * to allow this Hive engine to use bitboards
 *
 * The Bitboard object is dynamically allocated, with many different individual boards
 * strung together create one large object if necessary. Common numerical operations
 * are wrapped in functions like xorWith and unionWith. Overflow is dealt with by 
 * putting the bits back on the board in diffent locations, so bits are never discarded via
 * overflow. When a board is set to 0, calling pruneCache() will remove it from the Bitboard.
 *
 * Here are some cases in which bitboard usage speeds up the work:
 *		-Shifting several pieces in a certain direction
 *		-Finding the perimeter surrounding several pieces
 *		-Breath First Search
 *		-Calculating heuristics 
 *		-Calculating moves of a piece in a given state
 *		TODO: actually make these faster lol
 *		-Determining whether a Hive "gate" exists at a certain location
 *		-Checking whether a position has ever been visited before
 *		TODO: implement zobrist hashing like so
 *			-one number per piece per color per height per boardIndex per square
 *			-one number to represent side to move
 * 
 * Here are some sacrifices made:
 *		-Complicated and highly technical code
 *		-Higher memory requirment for performance
 *		-Separating a group of pieces into individual pieces is slow
 *		-Calculating distance between two nodes requires searching the entire hive
 *
 *
 */
#include <iterator>
#include <list>
#include <algorithm>
#include "constants.h"
#include "Bitboard.h"
std::random_device rd;     //Get a random seed from the OS entropy device, or whatever
std::mt19937_64 eng(rd()); //Use the 64-bit Mersenne Twister 19937 generator
                             //and seed it with entropy.

std::uniform_int_distribution<unsigned long long> distr;
using namespace std;
list <Direction> hexagonalDirections = {
	Direction::NE,
	Direction::E,
	Direction::SE,
	Direction::SW,
	Direction::W,
	Direction::NW
};


unordered_map <Direction, vector<int>> parameters  = {
	{Direction::E, {1, 1, 1}},
	{Direction::W, {0, 1, -1}},
	{Direction::S, {1, 8, BITBOARD_ROWS}},
	{Direction::N, {0, 8, -BITBOARD_COLS}}
};

unordered_map <Direction, unsigned long long> overflowBitmask =
{
	{Direction::W, 0xfefefefefefefefeu},
	{Direction::S, 0xffffffffffffff00u},
	{Direction::N, 0xffffffffffffffu},
	{Direction::E, 0x7f7f7f7f7f7f7f7fu}
};
unordered_map <Direction, vector<int>> overflowLocation =
{
	{Direction::E, {1,2,3,3,5,6,7,7,9,10,11,11,13,14,15,15}},
	{Direction::W, {0,0,1,2,4,4,5,6,8,8,9,10,12,12,13,14}},
	{Direction::S, {4,5,6,7,8,9,10,11,12,13,14,15,12,13,14,15}},
	{Direction::N, {0,1,2,3,0,1,2,3,4,5,6,7,8,9,10,11}}
};

Bitboard::Bitboard(unordered_map<int, unsigned long long > predefinedBoards){
	initialize(predefinedBoards);
}

/*
 * Iterate over a given map and set the specified board to the input
 */
void Bitboard::initialize(unordered_map < int, unsigned long long> predefinedBoards) {
	clear();
	//iterate through the map and update board internals
	for (auto keyValueList : predefinedBoards) {
		internalBoardCache.insert(keyValueList.first);
		internalBoards[keyValueList.first] = keyValueList.second;
	}
}


/*
 * Set this Bitboard to another
 */
void Bitboard::initializeTo(Bitboard &other) {
	clear();
	for (int i: other.internalBoardCache){
		internalBoardCache.insert(i);
		internalBoards[i] = other.internalBoards[i];
	}
}

/*
 * Set a specific board in this Bitboard
 */
void Bitboard::setBoard(int boardIndex, unsigned long long board) {
	internalBoards[boardIndex] = board;
	internalBoardCache.insert(boardIndex);
}
 
/*
 *
 * Algorithm designed to facilitate 2D translation across a Bitboard.
 *
 * Call this after a pure 2D shift so that the Bitboard maintains the
 * "Odd-r hexagonal" layout detailed here:
 *
 * https://www.redblobgames.com/grids/hexagons/#coordinates-offset
 *
 */
void Bitboard::convertToHexRepresentation ( Direction lastMovedDir , int lastMovedTimes) {

	unsigned long long changedBoards;

	//only have to adjust things if the move was even
	if (lastMovedTimes % 2){
		
		//if moving east, adjust the odd rows
		//else adjust the even rows
		if (lastMovedDir == Direction::NE || lastMovedDir == Direction::SE)
			changedBoards = ~ODD_ROWS_BITMASK;
		else
			changedBoards = ODD_ROWS_BITMASK;

		Bitboard changedBitboard;
		Bitboard unchangedBitboard;

		for (int i : internalBoardCache) {
			changedBitboard.setBoard(i, changedBoards &  internalBoards[i]);
			unchangedBitboard.setBoard(i, ~changedBoards &  internalBoards[i]);
		}

		//Shift certain rows E or W depending on which boards were adjusted
		if (changedBoards == ~ODD_ROWS_BITMASK) {
			changedBitboard.shiftDirection(Direction::E);
			changedBitboard.unionWith(unchangedBitboard);
		} else {
			changedBitboard.shiftDirection(Direction::W);
			changedBitboard.unionWith(unchangedBitboard);
		}

		initializeTo(changedBitboard);
	}
}


/*
 * Function that handles overflow and low-level bit operations
 * when shifting in an orthogonal direction.
 *
 * Does not preserve "Odd-r hexagonal" layout
 */
void Bitboard::shiftOrthogonalDirection(Direction dir, int numTimes){
	//assumes that dir is orthogonal

	int overflowAmount = numTimes % BITBOARD_WIDTH;
	int newHighBoardIndex, newLowBoardIndex;
	unsigned long long int overflowLow, overflowHigh, overflowLowMask, overflowHighMask;

	bool isAscendingDirection = parameters[dir][0];
	int boardIndexDiff = parameters[dir][2];

	vector <int> activeBoards;
	for (int i: internalBoardCache) {
		activeBoards.push_back(i);
	}

	if (isAscendingDirection) {
		std::reverse(activeBoards.begin(), activeBoards.end());
	}

	overflowLowMask = createLowOverflowMask(dir, overflowAmount);
	overflowHighMask = ~overflowLowMask;
	
	int boardLengthDiff = (numTimes / BITBOARD_WIDTH);

	for (int initialBoardIndex: activeBoards) {	

		//determine where all the bits will move to
		newLowBoardIndex =  initialBoardIndex + boardIndexDiff * boardLengthDiff;
		newHighBoardIndex = newLowBoardIndex + boardIndexDiff;

		//if there is no overflow
		//bits will not be spread out over two boards
		if (overflowAmount == 0) {
			newHighBoardIndex -= boardIndexDiff;
		}

		//wrap the board around to a legalBoard
		newHighBoardIndex = modulo(newHighBoardIndex, BITBOARD_SIZE);
		newLowBoardIndex = modulo(newLowBoardIndex, BITBOARD_SIZE);

		if (numTimes >= BITBOARD_HEIGHT) {
			internalBoardCache.erase(initialBoardIndex);
		}

		//get the two parts of the new mask
		overflowHigh = overflowHighMask & internalBoards[initialBoardIndex];
		overflowLow = overflowLowMask & internalBoards[initialBoardIndex];

		//shift the contents into the correct place
		overflowLow = adjustOverflowMask(dir, overflowAmount, true, overflowLow);
		overflowHigh = adjustOverflowMask(dir, overflowAmount, false, overflowHigh);


		internalBoards[newLowBoardIndex] = 0;
		internalBoards[newLowBoardIndex] |= overflowLow;
		internalBoardCache.insert(newLowBoardIndex);


		if (internalBoardCache.find(newHighBoardIndex) == internalBoardCache.end()) 
			internalBoards[newHighBoardIndex] = 0;

		internalBoards[newHighBoardIndex] |= overflowHigh;
		internalBoardCache.insert(newHighBoardIndex);
	}
}


/*
 * Takes any direction and shifts the board in that direction numTimes
 * 
 * When called, the Bitboard will shift into a direction represented by 
 * "Odd-r hexagonal" notation detailed here:
 *
 * https://www.redblobgames.com/grids/hexagons/#coordinates-offset
 *
 */
void Bitboard::shiftDirection(Direction dir, int numTimes){
	if (numTimes < 0) {
		dir = oppositeDirection[dir];
		numTimes = -numTimes;
	}
	if (dir == Direction::E|| dir == Direction::W||
		dir == Direction::S|| dir == Direction::N) {
		shiftOrthogonalDirection(dir, numTimes);
		pruneCache();
		return;
	}

	if (numTimes/2) {
		if (dir == Direction::NW || dir == Direction::SW) {
			shiftOrthogonalDirection(Direction::W, numTimes/2);
		} else {
			//if direction == SE or NE
			shiftOrthogonalDirection(Direction::E, numTimes/2);
		}
	}

	if (dir == Direction::SE || dir == Direction::SW) {
		shiftOrthogonalDirection(Direction::S, numTimes);
	} else {
		//if dir == NW or NE
		shiftOrthogonalDirection(Direction::N, numTimes);
	}

	convertToHexRepresentation(dir, numTimes);
	pruneCache();
}

//optimized (and ugly) code
//I'm sorry world =(
/*
 * Performs the same operation as shiftDirection(dir, 1) 
 * but is ~3x-4x faster
 *
 * does this by applying low level bit operations on a case-by-case basis
 */
void Bitboard::shiftDirection(Direction dir) {
	unsigned long long currentBoard;
	unsigned long long newBoard;

	unordered_map <int , unsigned long long > finalBoards;
	for (int boardIndex: internalBoardCache) {

		currentBoard = internalBoards[boardIndex];

		switch( dir) {
			case W:
				{

					int newHighBoardIndex;
					unsigned long long newHighBoard;
					newHighBoard= 0x101010101010101u & currentBoard;
					newHighBoard <<= 7;

					newBoard = 0xfefefefefefefefeu & currentBoard;
					newBoard >>= 1;


					if (newHighBoard) {
						newHighBoardIndex = (boardIndex) ? boardIndex - 1: 
							BITBOARD_SIZE - 1;
						if (finalBoards.find(newHighBoardIndex) == finalBoards.end())
							finalBoards[newHighBoardIndex] = newHighBoard;
						else {
							finalBoards[newHighBoardIndex] |= newHighBoard;
						}
					}
					if (newBoard) {
						if (finalBoards.find(boardIndex) == finalBoards.end() )
							finalBoards[boardIndex] = newBoard;
						else 
							finalBoards[boardIndex] |= newBoard;
					}
					break;
				}
			case N:
				{
					int newHighBoardIndex;
					unsigned long long newHighBoard;
					newHighBoard = 0xff00000000000000u & currentBoard;
					newHighBoard >>= 56;

					newBoard = 0xffffffffffffffu;
					newBoard &= currentBoard;
					newBoard <<= 8;

					if (newHighBoard) {
						newHighBoardIndex = modulo((boardIndex - BITBOARD_COLS) ,
								BITBOARD_SIZE);
						if (finalBoards.find(newHighBoardIndex) == finalBoards.end())
							finalBoards[newHighBoardIndex] = newHighBoard;
						else {
							finalBoards[newHighBoardIndex] |= newHighBoard;
						}
					}
					if (newBoard){
						if (finalBoards.find(boardIndex) == finalBoards.end() )
							finalBoards[boardIndex] = newBoard;
						else 
							finalBoards[boardIndex] |= newBoard;
					}
					break;
				}
			case E:
				{
					int newHighBoardIndex;
					unsigned long long newHighBoard;
					newHighBoard= 0x8080808080808080u & currentBoard;
					newHighBoard >>= 7;

					newBoard = 0x7f7f7f7f7f7f7f7fu & currentBoard;
					newBoard <<= 1;

					if (newHighBoard) {
						newHighBoardIndex = (boardIndex + 1) % BITBOARD_SIZE;
						if (finalBoards.find(newHighBoardIndex) == finalBoards.end())
							finalBoards[newHighBoardIndex] = newHighBoard;
						else {
							finalBoards[newHighBoardIndex] |= newHighBoard;
						}
					}
					if (newBoard){
						if (finalBoards.find(boardIndex) == finalBoards.end() )
							finalBoards[boardIndex] = newBoard;
						else 
							finalBoards[boardIndex] |= newBoard;
					}	
					break;
				} 
			case S:
				{
					int newHighBoardIndex;
					unsigned long long newHighBoard;
					newHighBoard= 0xff & currentBoard;
					newHighBoard <<= 56;

					newBoard = 0xffffffffffffff00u & currentBoard;
					newBoard >>= 8;

					if (newHighBoard) {
						newHighBoardIndex = (boardIndex + BITBOARD_COLS) %
							BITBOARD_SIZE;
						if (finalBoards.find(newHighBoardIndex) == finalBoards.end())
							finalBoards[newHighBoardIndex] = newHighBoard;
						else {
							finalBoards[newHighBoardIndex] |= newHighBoard;
						}
					}
					if (newBoard){
						if (finalBoards.find(boardIndex) == finalBoards.end() )
							finalBoards[boardIndex] = newBoard;
						else 
							finalBoards[boardIndex] |= newBoard;
					}	
					break;

				}

			case NW: 
				{
					unsigned long long newVerticalBoard, newHorizontalBoard, intermediate;
					int newVerticalBoardIndex, newHorizontalBoardIndex;

					newHorizontalBoard = 0xff00000000000000u & currentBoard;
					newHorizontalBoard >>= 56;

					newVerticalBoard = 0x1000100010001u & currentBoard;
					newVerticalBoard <<= 15;

					intermediate = 0xfe00fe00fe00feu & currentBoard;
					newBoard = intermediate << 7;
					intermediate = 0xff00ff00ff00u & currentBoard;
					newBoard |= intermediate << 8;

					if (newVerticalBoard) {
						newVerticalBoardIndex = (boardIndex) ? boardIndex - 1: 
							BITBOARD_SIZE - 1;
						if (finalBoards.find(newVerticalBoardIndex) == finalBoards.end())
							finalBoards[newVerticalBoardIndex] = newVerticalBoard;
						else {
							finalBoards[newVerticalBoardIndex] |= newVerticalBoard;
						}
					}


					if (newHorizontalBoard) {
						newHorizontalBoardIndex = modulo (boardIndex - BITBOARD_COLS,
								BITBOARD_SIZE);
						if (finalBoards.find(newHorizontalBoardIndex) == finalBoards.end())
							finalBoards[newHorizontalBoardIndex] = newHorizontalBoard;
						else {
							finalBoards[newHorizontalBoardIndex] |= newHorizontalBoard;
						}
					}
					if (newBoard){
						if (finalBoards.find(boardIndex) == finalBoards.end() )
							finalBoards[boardIndex] = newBoard;
						else 
							finalBoards[boardIndex] |= newBoard;
					}	
					break;
				}
			case SW:
				{
					int newVerticalBoardIndex, newHorizontalBoardIndex, newDiagBoardIndex;
					unsigned long long newVerticalBoard, newHorizontalBoard,
								  intermediate, newDiagBoard;
					newDiagBoard = (1 & currentBoard)? 0x8000000000000000u: 0;

					newVerticalBoard = 0x1000100010000u & currentBoard;
					newVerticalBoard >>= 1;

					newHorizontalBoard = 254u & currentBoard;
					newHorizontalBoard <<= 55;

					intermediate = 0xfe00fe00fe0000u & currentBoard;
					newBoard = (intermediate >> 9);
					intermediate = 0xff00ff00ff00ff00u & currentBoard;
					newBoard |= (intermediate >> 8);


					if (newDiagBoard) {
						newDiagBoardIndex = (boardIndex - 1 + BITBOARD_COLS) %
							BITBOARD_SIZE;
						if (finalBoards.find(newDiagBoardIndex) == finalBoards.end())
							finalBoards[newDiagBoardIndex] = newDiagBoard;
						else {
							finalBoards[newDiagBoardIndex] |= newDiagBoard;
						}
					}
					if (newVerticalBoard) {
						newVerticalBoardIndex = (boardIndex) ? boardIndex - 1:
							BITBOARD_SIZE - 1;
						if (finalBoards.find(newVerticalBoardIndex) == finalBoards.end())
							finalBoards[newVerticalBoardIndex] = newVerticalBoard;
						else {
							finalBoards[newVerticalBoardIndex] |= newVerticalBoard;
						}
					}
					if (newHorizontalBoard) {
						newHorizontalBoardIndex = (boardIndex + BITBOARD_COLS) %
							BITBOARD_SIZE;
						if (finalBoards.find(newHorizontalBoardIndex) == finalBoards.end())
							finalBoards[newHorizontalBoardIndex] = newHorizontalBoard;
						else {
							finalBoards[newHorizontalBoardIndex] |= newHorizontalBoard;
						}
					}
					if (newBoard){
						if (finalBoards.find(boardIndex) == finalBoards.end() )
							finalBoards[boardIndex] = newBoard;
						else 
							finalBoards[boardIndex] |= newBoard;
					}	
					break;
				}

			case NE:
				{
					unsigned long long newVerticalBoard, newHorizontalBoard,
								  intermediate;
					int newVerticalBoardIndex, newHorizontalBoardIndex, newDiagBoardIndex, 
						newDiagBoard;
					newDiagBoard = (0x8000000000000000u & currentBoard)? 1: 0;

					newVerticalBoard = 0x800080008000u & currentBoard;
					newVerticalBoard <<= 1;

					newHorizontalBoard =0x7f00000000000000u & currentBoard;
					newHorizontalBoard >>= 55;

					intermediate = 0x7f007f007f00u & currentBoard;
					newBoard = (intermediate << 9);
					intermediate = 0xff00ff00ff00ffu & currentBoard;
					newBoard |= (intermediate << 8);

					if (newDiagBoard) {
						newDiagBoardIndex = modulo (boardIndex + 1 - BITBOARD_COLS, 
								BITBOARD_SIZE);
						if (finalBoards.find(newDiagBoardIndex) == finalBoards.end())
							finalBoards[newDiagBoardIndex] = newDiagBoard;
						else {
							finalBoards[newDiagBoardIndex] |= newDiagBoard;
						}
					}
					if (newVerticalBoard) {
						newVerticalBoardIndex = (boardIndex + 1) % BITBOARD_SIZE;
						if (finalBoards.find(newVerticalBoardIndex) == finalBoards.end())
							finalBoards[newVerticalBoardIndex] = newVerticalBoard;
						else {
							finalBoards[newVerticalBoardIndex] |= newVerticalBoard;
						}
					}
					if (newHorizontalBoard) {
						newHorizontalBoardIndex = modulo(boardIndex - BITBOARD_COLS,
								BITBOARD_SIZE);
						if (finalBoards.find(newHorizontalBoardIndex) == finalBoards.end())
							finalBoards[newHorizontalBoardIndex] = newHorizontalBoard;
						else {
							finalBoards[newHorizontalBoardIndex] |= newHorizontalBoard;
						}
					}
					if (newBoard){
						if (finalBoards.find(boardIndex) == finalBoards.end() )
							finalBoards[boardIndex] = newBoard;
						else 
							finalBoards[boardIndex] |= newBoard;
					}	
					break;
				}
			case SE:
				{
					unsigned long long newVerticalBoard, newHorizontalBoard, intermediate;
					int newVerticalBoardIndex, newHorizontalBoardIndex;

					newHorizontalBoard = 0xff & currentBoard;
					newHorizontalBoard <<= 56;

					newVerticalBoard = 0x8000800080008000u & currentBoard;
					newVerticalBoard >>= 15;

					intermediate = 0x7f007f007f007f00u & currentBoard;
					newBoard = intermediate >> 7;
					intermediate = 0xff00ff00ff0000u & currentBoard;
					newBoard |= intermediate >> 8;

					if (newVerticalBoard) {
						newVerticalBoardIndex = (boardIndex + 1) % BITBOARD_SIZE;
						if (finalBoards.find(newVerticalBoardIndex) == finalBoards.end())
							finalBoards[newVerticalBoardIndex] = newVerticalBoard;
						else {
							finalBoards[newVerticalBoardIndex] |= newVerticalBoard;
						}
					}
					if (newHorizontalBoard) {
						newHorizontalBoardIndex = (boardIndex + BITBOARD_COLS) % 
							BITBOARD_SIZE;
						if (finalBoards.find(newHorizontalBoardIndex) == finalBoards.end())
							finalBoards[newHorizontalBoardIndex] = newHorizontalBoard;
						else {
							finalBoards[newHorizontalBoardIndex] |= newHorizontalBoard;
						}
					}
					if (newBoard){
						if (finalBoards.find(boardIndex) == finalBoards.end() )
							finalBoards[boardIndex] = newBoard;
						else 
							finalBoards[boardIndex] |= newBoard;
					}	
					break;

				}
		}
	}

	internalBoardCache.clear();
	for (auto iter: finalBoards) {
		internalBoards[iter.first] = iter.second;
		internalBoardCache.insert(iter.first);
	}
}

/*
 * function used to deal with overflow
 *
 * Direction dir : orthogonal direction (N,W,E,S)
 *
 * Returns a bitmask that contains the possible remaining bits on a board
 * after shifting in an orthogonal direction
 *
 */
unsigned long long Bitboard::createLowOverflowMask(Direction dir, int overflowAmount) {
	long long overflowLow; 
	//assumes orthogonal direction passed in

	if (overflowAmount == 0) {
		return -1;
	} 

	overflowLow = overflowBitmask[dir];
	int shiftMultiplier = parameters[dir][1];

	//TODO: just store the possible masks to prevent recalculation
	for (int i = 0; i< overflowAmount - 1; i++) {
		if (dir == Direction::W || dir == Direction::S){
			overflowLow &= (overflowLow << shiftMultiplier);
		} else {
			overflowLow &= (overflowLow >> shiftMultiplier);
		}
	}

	return overflowLow;	
}

/*
 * function used to deal with overflow
 *
 * Direction dir : orthogonal direction (N,W,E,S)  
 * int overflowAmount : # of rows (or cols) of set bits that the highOverflowMask contains
 * bool low : true if overflowMask is a lowOverflowMask, false if highOverflowMask
 * overflowMask : input to operate on
 *
 * Returns a board has its bits translated to the other end of a board
 * for example:
 *
 * lowOverflowMask = 
 *     1 1 1 0 0 0 0 0
 *     1 1 1 0 0 0 0 0
 *     1 1 1 0 0 0 0 0
 *          ...
 *
 * adjustOverflowMask(Direction::E, 5, true, lowOverflowMask)
 *     0 0 0 0 0 1 1 1
 *     0 0 0 0 0 1 1 1
 *     0 0 0 0 0 1 1 1
 *          ...
 */
unsigned long long Bitboard::adjustOverflowMask(
		Direction dir, int overflowAmount, bool low, unsigned long long overflowMask) {

	int shiftMultiplier = parameters[dir][1];
	if (dir == Direction::E || dir == Direction::N){
		if (low)
			overflowMask <<= overflowAmount*shiftMultiplier;
		else 
			overflowMask >>= (BITBOARD_WIDTH - overflowAmount)*shiftMultiplier;
	} else {
		if (low)
			overflowMask >>= overflowAmount*shiftMultiplier;
		else 
			overflowMask <<= (BITBOARD_WIDTH - overflowAmount)*shiftMultiplier;
	}
	
	return overflowMask;
}

/* 
 * Finds all adjecent hexagonal neighbors in *this that have not yet been visited
 * and update frontier to them
 *
 * Bitboard frontier : mutated to new set of adjacent neighbors
 * Bitboard visited : mutated to all previously visited neighbors
 */
void Bitboard::floodFillStep(Bitboard &frontier,  Bitboard &visited){
	//assumes that everything in frontier is a legal node

	//visited = legal nodes that have already been traversed
	visited.unionWith(frontier);

	//get connecting nodes
	Bitboard perimeter = frontier.getPerimeter();

	//keep only traversable nodes
	perimeter.intersectionWith(*this);
	frontier.initializeTo(perimeter);

	//nodes in visited are not in frontier
	frontier.notIntersectionWith(visited);
}

/*
 * Perform a Breath First Search on the graph G
 * where edges in G are all hexagonal directions
 * and nodes in G are bits set in *this Bitboard
 *
 * Bitboard frontier : start nodes;
 * 
 */
void Bitboard::floodFill(Bitboard &frontier){
	//assumes frontier is a legal node

	Bitboard visited;
	while (frontier.internalBoardCache.size()) {
		floodFillStep(frontier, visited);
		frontier.pruneCache();
	}

	//replace frontier with all nodes found
	frontier.initializeTo(visited);
}

/*
 * Returns true if the other Bitboard is equal to this one
 */
bool Bitboard::equals(Bitboard& other){
	//TODO: fix prunce cache leaks
	other.pruneCache();
	pruneCache();
	boost::container::flat_set<int> combined;
	for (auto a : other.internalBoardCache){
		combined.insert(a);
	}
	for (auto a : internalBoardCache){
		combined.insert(a);
	}
	if (combined.size() != other.internalBoardCache.size() ||
		combined.size() != internalBoardCache.size())
		return false;
	
	for (auto i: combined){
		if (internalBoards[i] != other.internalBoards[i]) return false;
	}
	return true;
}

/*
 * Returns the smallest bit found on the lowest numbered board
 */
const pair <const int , const unsigned long long >
Bitboard::getLeastSignificantBit () const {
	pair <int, unsigned long long> LSB{0,0};
	int min = 12345678;
	for (int i: internalBoardCache) 
		min = (min < i) ? min : i;
	if (min == 12345678)
		return LSB;
	return {min, internalBoards[min] & -internalBoards[min]};
}

/*
 * Removes any board that has 0 bits set from the cache
 *
 * Causes significant speed up on certain tasks
 */
void Bitboard::pruneCache(){
	list <int> emptyBoards;

	for (int i: internalBoardCache){
		if (internalBoards[i] == 0){
			emptyBoards.push_front(i);
		}
	}

	for (int i: emptyBoards) {
		internalBoardCache.erase(i);
	}
}

/*
 * Applies the union ( | ) operator on all boards in this Bitboard
 */
void Bitboard::unionWith( Bitboard &other){
	for (auto i: other.internalBoardCache){
		if (internalBoardCache.find(i) == internalBoardCache.end()) internalBoards[i] = 0;
		internalBoardCache.insert(i);
		internalBoards[i] |= other.internalBoards[i];
	}
}

/*
 * Applies the intersection ( & ) operator on all boards in this Bitboard
 */
void Bitboard::intersectionWith( Bitboard &other) {
	for (auto i: internalBoardCache){
		if (other.internalBoardCache.find(i) != other.internalBoardCache.end())
			internalBoards[i] &= other.internalBoards[i];
		else {
			internalBoards[i] = 0;
		}
	}
	pruneCache();
}

/*
 * Applies the xor ( ^ ) operator on all boards in this Bitboard
 */
void Bitboard::xorWith( Bitboard &other) {
	for (auto i: other.internalBoardCache) {
		if (internalBoardCache.find(i) == internalBoardCache.end()) internalBoards[i] = 0;
		internalBoards[i] ^= other.internalBoards[i];
		internalBoardCache.insert(i);
	}
	pruneCache();
}

/*
 * Applies the intersection ( & ) operator on the complement of all boards in this Bitboard
 */
void Bitboard::notIntersectionWith( Bitboard &other) {
	for (auto i: other.internalBoardCache) {
		internalBoards[i] &= ~other.internalBoards[i];
	}
}

/*
 * Returns true if any bit from the other Bitboard exists in this Bitboard
 */
bool Bitboard::containsAny(Bitboard& other) {
	for (int i: other.internalBoardCache) { 
		if (internalBoardCache.find(i) != internalBoardCache.end())
			if (internalBoards[i] & other.internalBoards[i]) 
				return true;
	}
	return false;
}

void Bitboard::clear() {
	internalBoardCache.clear();
}

/*
 * Mutate this board to be copy-pasted into specified directions
 */
void Bitboard::duplicateBoard(list <Direction> dirs){
	Bitboard init;
	Bitboard duplicated;
	duplicated.initializeTo(*this);


	for (Direction dir: dirs) { 
		
		init.initializeTo(*this);
		init.shiftDirection(dir);
		duplicated.unionWith(init);

	}

	initializeTo(duplicated);
}

int Bitboard::count() const{
	int total = 0;
	for (int i: internalBoardCache) {
		total += __builtin_popcountll(internalBoards[i]);
	}
	return total;
}


// optimized so it's very ugly =[
/*
 *  Returns all adjecent hexagonal neighbors of all bits in this Bitboard
 *	where a neighbor IS NOT already in this Bitboard
 *  
 *  about ~2x faster than slowGetPerimeter()
 */
Bitboard  Bitboard::getPerimeter() {
	Bitboard perimeter;

	for (auto boardIndex: internalBoardCache) {
		unsigned long long currentBoard = internalBoards[boardIndex];
		int count = __builtin_popcountll(currentBoard);
		if (!count) continue;
		if (count > PERIMETER_SIZE 
		    || PERIMETER[count].find(currentBoard) == PERIMETER[count].end()){
			//if did not find in hash table; default to 
			//resolving bits indiviually
			count = 1;
		} 
		

		unsigned long long*  hashedBoards ;
		unsigned long long leastSignificantBit;
		while ( currentBoard) {
			if (count == 1) {
				leastSignificantBit = (currentBoard & -currentBoard);
				currentBoard ^= leastSignificantBit;
				hashedBoards = PERIMETER[count][leastSignificantBit];
			} else {
				hashedBoards = PERIMETER[count][currentBoard];
				currentBoard = 0;
			}

			if (hashedBoards[0])
				//assign the orginalBoard
				perimeter.unionWith(boardIndex , hashedBoards[0]);

			//if there is overflow
			if (hashedBoards[4]) { 
				switch( hashedBoards[4]) {
					case 1:
						//Upper and Right Boards are assigned

						if (hashedBoards[1]) 
							perimeter.unionWith(modulo((boardIndex - BITBOARD_COLS),  
										BITBOARD_SIZE), hashedBoards[1]);
						if (hashedBoards[2]) 
							perimeter.unionWith((boardIndex + 1) % BITBOARD_SIZE,
									hashedBoards[2]);
						if (hashedBoards[3]) 
							perimeter.unionWith(modulo((boardIndex + 1 - BITBOARD_COLS)
										,BITBOARD_SIZE), hashedBoards[3]);
						break;

					case 2:
						//Lower and Right Boards are assigned
						if (hashedBoards[1]) 
							perimeter.unionWith((boardIndex + BITBOARD_COLS) % 
									BITBOARD_SIZE, hashedBoards[1]);
						if (hashedBoards[2]) 
							perimeter.unionWith((boardIndex + 1) % BITBOARD_SIZE,
									hashedBoards[2]);
						if (hashedBoards[3]) 
							perimeter.unionWith((boardIndex + 1 + BITBOARD_COLS)
									% BITBOARD_SIZE, hashedBoards[3]);
						break;

					case 3:
						//Lower and Left Boards are assigned
						if (hashedBoards[1]) 
							perimeter.unionWith((boardIndex + BITBOARD_COLS) % 
									BITBOARD_SIZE, hashedBoards[1]);
						if (hashedBoards[2]) 
							perimeter.unionWith(modulo((boardIndex - 1),BITBOARD_SIZE),
									hashedBoards[2]);
						if (hashedBoards[3]) 
							perimeter.unionWith((boardIndex - 1 + BITBOARD_COLS)
									% BITBOARD_SIZE, hashedBoards[3]);
						break;
					case 4:
						//Upper and Left Boards are assigned
						if (hashedBoards[1]) 
							perimeter.unionWith(modulo((boardIndex - BITBOARD_COLS),  
										BITBOARD_SIZE), hashedBoards[1]);
						if (hashedBoards[2]) 
							perimeter.unionWith(modulo((boardIndex - 1),BITBOARD_SIZE),
									hashedBoards[2]);
						if (hashedBoards[3]) 
							perimeter.unionWith(modulo((boardIndex - 1 - BITBOARD_COLS)
										,BITBOARD_SIZE), hashedBoards[3]);
						break;

					default:
						cout << "unexpected value for PERIMETER hash table" << endl;
						throw 69;
				}
			}
		}
	}
	perimeter.notIntersectionWith(*this);
	return perimeter;
}

/*
 *  Returns all adjecent hexagonal neighbors of all bits in this Bitboard
 *	where a neighbor IS NOT already in this Bitboard
 *  
 *  Used to generate getPerimeterHashTable
 *
 *	about ~2x slower than getPerimeter()
 */
Bitboard Bitboard::slowGetPerimeter(){
	Bitboard perimeter;
	perimeter.initializeTo(*this);
	perimeter.duplicateBoard( hexagonalDirections);	
	perimeter.xorWith(*this);
	return perimeter;
}

/*
 *  Applies the union operator ( | ) on a specified board in this Bitboard
 */
void Bitboard::unionWith(int boardIndex, unsigned long long board) {
	if (internalBoardCache.find(boardIndex) == internalBoardCache.end()) {
		internalBoards[boardIndex] = board;
		internalBoardCache.insert(boardIndex);
	} else {
		internalBoards[boardIndex] |= board;
	}
}

/*
 * Returns a map of all the bits that were set and which board it was set on
 */
unordered_map< int, vector < unsigned long long >> Bitboard::split(){
	unordered_map< int, vector <unsigned long long >> returnMap;

	unsigned long long board, leastSignificantBit;
	for (int i: internalBoardCache){
		board = internalBoards[i];
		while (board) {
			leastSignificantBit = board & -board;
			returnMap[i].push_back(leastSignificantBit);
			board ^= leastSignificantBit; // remove least significant bit
		}
	}
	return returnMap;
}

/*
 * Returns a list of all individual bits from this Bitboard wrapped in a new Bitboard
 */
list <Bitboard> Bitboard::splitIntoBitboards() {
	list <Bitboard> returnList;
	for (int i: internalBoardCache){
		unsigned long long board = internalBoards[i];
		while(board) {
			unsigned long long leastSignificantBit = board & -board;
			board ^= leastSignificantBit; // remove least significant bit

			Bitboard splitted;
			splitted.setBoard(i, leastSignificantBit);
			returnList.push_front(splitted);
		}		
	}
	return returnList;
}

/*
 * Returns an iterator that points to a list generated by splitIntoBitboards()
 */
list <Bitboard>::const_iterator Bitboard::begin() {
	const auto & bitboards = splitIntoBitboards();
	return bitboards.begin();
}

/*
 * Returns a list of all connected components from this Bitboard wrapped in a new Bitboard
 */
vector <Bitboard> Bitboard::splitIntoConnectedComponents(){
	vector <Bitboard> components;

	Bitboard boards;
	boards.initializeTo(*this);

	while (boards.internalBoardCache.size() != 0) {
		int boardIndex = *(boards.internalBoardCache.begin());
		unsigned long long * currentBoard = &boards.internalBoards[boardIndex];

		 while (*currentBoard) {
		    //repeat if there are more points on this board

			//pick some starting node
			unsigned long long leastSignificantBit = *currentBoard & -*currentBoard;
			Bitboard returnBitboard({{boardIndex, leastSignificantBit}});

			//BFS starting at that node
			boards.floodFill(returnBitboard);

			components.push_back(returnBitboard);

			//Remove found nodes
			boards.xorWith(returnBitboard);

		}
		
		//delete now empty board
		boards.internalBoardCache.erase(boardIndex);
	}

	return components;
}

void Bitboard::print() {
	for (int i: internalBoardCache) {
		cout << i << "\t"  << internalBoards[i] << endl;
	}	

	if (!internalBoardCache.size())  cout << "empty" << endl;
}

/*
 * Returns a hash for a Bitboard with A SINGLE set bit 
 */
int Bitboard::hash() {
	return *(internalBoardCache.begin()) | (__builtin_clzll(internalBoards[
				*(internalBoardCache.begin())]) << 8);
}

int Bitboard::getRandomBoardIndex() {
	int total = count();
	total = std::rand() % total;
	int boardSelect = 0;
	for (int i : internalBoardCache) {	
		boardSelect += __builtin_popcountll(internalBoards[i]);
		if (boardSelect > total) return i;
	}
	return -1000;
}

/*
 * Returns a random set bit from this Bitboard wrapped in a new Bitboard
 */
Bitboard Bitboard::getRandom() {
	int randomBoardIndex = getRandomBoardIndex();
	unsigned long long s = internalBoards[randomBoardIndex];
	unsigned long long t;
	//get a random bit from given bitboard
	while( (s & (s - 1))) {
		t = s & dist(e2);
		s = (t) ? t : s;
	}

	return Bitboard({{randomBoardIndex, s}});
}
