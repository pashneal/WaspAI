#include <iterator>
#include <list>
#include <set>
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
	{Direction::S, {1, 8, BITBOARD_CONTAINER_ROWS}},
	{Direction::N, {0, 8, -BITBOARD_CONTAINER_COLS}}
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

BitboardContainer::BitboardContainer(unordered_map<int, unsigned long long > predefinedBoards){
	initialize(predefinedBoards);
}

void BitboardContainer::initialize(unordered_map < int, unsigned long long> predefinedBoards) {
	clear();
	//iterate through the map and update board internals
	for (auto keyValueList : predefinedBoards) {
		internalBoardCache.insert(keyValueList.first);
		internalBoards[keyValueList.first] = keyValueList.second;
	}
}

void BitboardContainer::initializeTo(BitboardContainer &other) {
	clear();
	for (int i: other.internalBoardCache){
		internalBoardCache.insert(i);
		internalBoards[i] = other.internalBoards[i];
	}
}

//TODO: make internalBoards private so you have to use this
void BitboardContainer::setBoard(int boardIndex, unsigned long long board) {
	internalBoards[boardIndex] = board;
	internalBoardCache.insert(boardIndex);
}
 
void BitboardContainer::convertToHexRepresentation (
		Direction lastMovedDir , int lastMovedTimes) {

	unsigned long long changedBoards;


	if (lastMovedTimes % 2){
		//only have to adjust things if the move was even
		
		//if moving east, adjust the odd rows
		//else adjust the even rows
		if (lastMovedDir == Direction::NE || lastMovedDir == Direction::SE)
			changedBoards = ~ODD_ROWS_BITMASK;
		else
			changedBoards = ODD_ROWS_BITMASK;

		BitboardContainer changedBitboardContainer;
		BitboardContainer unchangedBitboardContainer;

		for (int i : internalBoardCache) {

			changedBitboardContainer.setBoard(i, changedBoards &  internalBoards[i]);
			unchangedBitboardContainer.setBoard(i, ~changedBoards &  internalBoards[i]);
			
		}


		if (changedBoards == ~ODD_ROWS_BITMASK) {
			changedBitboardContainer.shiftDirection(Direction::E);
			changedBitboardContainer.unionWith(unchangedBitboardContainer);
		} else {
			changedBitboardContainer.shiftDirection(Direction::W);
			changedBitboardContainer.unionWith(unchangedBitboardContainer);
		}

		initializeTo(changedBitboardContainer);
	}
}

//make this private
void BitboardContainer::shiftOrthogonalDirection(Direction dir, int numTimes){
	//assumes that dir is orthogonal
	//TODO: separate orthogonal and hexagonal directions


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


		//TODO: name everything here better
	
		//determine where all the bits will move to
		newLowBoardIndex =  initialBoardIndex + boardIndexDiff * boardLengthDiff;
		newHighBoardIndex = newLowBoardIndex + boardIndexDiff;

		//if there is no overflow
		//bits will not be spread out over two boards
		if (overflowAmount == 0) {
			newHighBoardIndex -= boardIndexDiff;
		}

		//wrap the board around to a legalBoard
		newHighBoardIndex = modulo(newHighBoardIndex, BITBOARD_CONTAINER_SIZE);
		newLowBoardIndex = modulo(newLowBoardIndex, BITBOARD_CONTAINER_SIZE);

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


void BitboardContainer::shiftDirection(Direction dir, int numTimes){
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
void BitboardContainer::shiftDirection(Direction dir) {
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
							BITBOARD_CONTAINER_SIZE - 1;
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
						newHighBoardIndex = modulo((boardIndex - BITBOARD_CONTAINER_COLS) ,
								BITBOARD_CONTAINER_SIZE);
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
						newHighBoardIndex = (boardIndex + 1) % BITBOARD_CONTAINER_SIZE;
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
						newHighBoardIndex = (boardIndex + BITBOARD_CONTAINER_COLS) %
							BITBOARD_CONTAINER_SIZE;
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
							BITBOARD_CONTAINER_SIZE - 1;
						if (finalBoards.find(newVerticalBoardIndex) == finalBoards.end())
							finalBoards[newVerticalBoardIndex] = newVerticalBoard;
						else {
							finalBoards[newVerticalBoardIndex] |= newVerticalBoard;
						}
					}


					if (newHorizontalBoard) {
						newHorizontalBoardIndex = modulo (boardIndex - BITBOARD_CONTAINER_COLS,
								BITBOARD_CONTAINER_SIZE);
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
						newDiagBoardIndex = (boardIndex - 1 + BITBOARD_CONTAINER_COLS) %
							BITBOARD_CONTAINER_SIZE;
						if (finalBoards.find(newDiagBoardIndex) == finalBoards.end())
							finalBoards[newDiagBoardIndex] = newDiagBoard;
						else {
							finalBoards[newDiagBoardIndex] |= newDiagBoard;
						}
					}
					if (newVerticalBoard) {
						newVerticalBoardIndex = (boardIndex) ? boardIndex - 1:
							BITBOARD_CONTAINER_SIZE - 1;
						if (finalBoards.find(newVerticalBoardIndex) == finalBoards.end())
							finalBoards[newVerticalBoardIndex] = newVerticalBoard;
						else {
							finalBoards[newVerticalBoardIndex] |= newVerticalBoard;
						}
					}
					if (newHorizontalBoard) {
						newHorizontalBoardIndex = (boardIndex + BITBOARD_CONTAINER_COLS) %
							BITBOARD_CONTAINER_SIZE;
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
						newDiagBoardIndex = modulo (boardIndex + 1 - BITBOARD_CONTAINER_COLS, 
								BITBOARD_CONTAINER_SIZE);
						if (finalBoards.find(newDiagBoardIndex) == finalBoards.end())
							finalBoards[newDiagBoardIndex] = newDiagBoard;
						else {
							finalBoards[newDiagBoardIndex] |= newDiagBoard;
						}
					}
					if (newVerticalBoard) {
						newVerticalBoardIndex = (boardIndex + 1) % BITBOARD_CONTAINER_SIZE;
						if (finalBoards.find(newVerticalBoardIndex) == finalBoards.end())
							finalBoards[newVerticalBoardIndex] = newVerticalBoard;
						else {
							finalBoards[newVerticalBoardIndex] |= newVerticalBoard;
						}
					}
					if (newHorizontalBoard) {
						newHorizontalBoardIndex = modulo(boardIndex - BITBOARD_CONTAINER_COLS,
								BITBOARD_CONTAINER_SIZE);
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
						newVerticalBoardIndex = (boardIndex + 1) % BITBOARD_CONTAINER_SIZE;
						if (finalBoards.find(newVerticalBoardIndex) == finalBoards.end())
							finalBoards[newVerticalBoardIndex] = newVerticalBoard;
						else {
							finalBoards[newVerticalBoardIndex] |= newVerticalBoard;
						}
					}
					if (newHorizontalBoard) {
						newHorizontalBoardIndex = (boardIndex + BITBOARD_CONTAINER_COLS) % 
							BITBOARD_CONTAINER_SIZE;
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

unsigned long long BitboardContainer::createLowOverflowMask(Direction dir, int overflowAmount) {
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

unsigned long long BitboardContainer::adjustOverflowMask(
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
void BitboardContainer::floodFillStep(BitboardContainer &frontier,  BitboardContainer &visited){
	//assumes that everything in frontier is a legal node

	//visited = legal nodes that have already been traversed
	visited.unionWith(frontier);

	//get connecting nodes
	BitboardContainer perimeter = frontier.getPerimeter();

	//keep only traversable nodes
	perimeter.intersectionWith(*this);
	frontier.initializeTo(perimeter);

	//nodes in visited are not in frontier
	frontier.unionWith(visited);
	frontier.xorWith(visited);
}



void BitboardContainer::floodFill(BitboardContainer &frontier){

	//assumes frontier is a legal node

	BitboardContainer visited;
	while (frontier.internalBoardCache.size()) {
		floodFillStep(frontier, visited);
		frontier.pruneCache();
	}

	//replace frontier with all nodes found
	frontier.initializeTo(visited);
}

bool BitboardContainer::equals(BitboardContainer& other){
	//TODO: fix prunce cache leaks
	other.pruneCache();
	pruneCache();
	set<int> combined;
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

pair <int , unsigned long long > BitboardContainer::getLeastSignificantBit () {
	pair <int, unsigned long long> LSB;
	int min = 40;
	for (int i: internalBoardCache) 
		min = (min < i) ? min : i;
	LSB.first = min;
	LSB.second = (internalBoards[min] &  -internalBoards[min]);

	return LSB;
}

void BitboardContainer::pruneCache(){
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

void BitboardContainer::unionWith( BitboardContainer &other){
	for (auto i: other.internalBoardCache){
		if (internalBoardCache.find(i) == internalBoardCache.end()) internalBoards[i] = 0;
		internalBoardCache.insert(i);
		internalBoards[i] |= other.internalBoards[i];
	}
}

void BitboardContainer::intersectionWith( BitboardContainer &other) {
	for (auto i: internalBoardCache){
		if (other.internalBoardCache.find(i) != other.internalBoardCache.end())
			internalBoards[i] &= other.internalBoards[i];
		else {
			internalBoards[i] = 0;
		}
	}
	pruneCache();
}

void BitboardContainer::xorWith( BitboardContainer &other) {
	for (auto i: other.internalBoardCache) {
		if (internalBoardCache.find(i) == internalBoardCache.end()) internalBoards[i] = 0;
		internalBoards[i] ^= other.internalBoards[i];
		internalBoardCache.insert(i);
	}
	pruneCache();
}

void BitboardContainer::notIntersectionWith( BitboardContainer &other) {
	for (auto i: other.internalBoardCache) {
		internalBoards[i] &= ~other.internalBoards[i];
	}
}

bool BitboardContainer::containsAny(BitboardContainer& other) {
	for (int i: other.internalBoardCache) { 
		if (internalBoardCache.find(i) != internalBoardCache.end())
			if (internalBoards[i] & other.internalBoards[i]) 
				return true;
	}
	return false;
}

void BitboardContainer::clear() {
	internalBoardCache.clear();
}

void BitboardContainer::duplicateBoard(list <Direction> dirs){
	BitboardContainer init;
	BitboardContainer duplicated;
	duplicated.initializeTo(*this);


	for (Direction dir: dirs) { 
		
		init.initializeTo(*this);
		init.shiftDirection(dir);
		duplicated.unionWith(init);

	}

	initializeTo(duplicated);
}

int BitboardContainer::count(){
	int total = 0;
	for (int i: internalBoardCache) {
		total += __builtin_popcountll(internalBoards[i]);
	}
	return total;
}


// optimized so its ugly =[
BitboardContainer  BitboardContainer::getPerimeter() {
	BitboardContainer perimeter;

	for (auto boardIndex: internalBoardCache) {
		unsigned long long currentBoard = internalBoards[boardIndex];
		int count = __builtin_popcountll(currentBoard);
		if (!count) continue;
		if (count > PERIMETER_SIZE 
		    || PERIMETER[count].find(currentBoard) == PERIMETER[count].end()){
			//if did not find in hash table; default to 
			//resolving bits indiviually
			count = 1;
		} else {
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
							perimeter.unionWith(modulo((boardIndex - BITBOARD_CONTAINER_COLS),  
										BITBOARD_CONTAINER_SIZE), hashedBoards[1]);
						if (hashedBoards[2]) 
							perimeter.unionWith((boardIndex + 1) % BITBOARD_CONTAINER_SIZE,
									hashedBoards[2]);
						if (hashedBoards[3]) 
							perimeter.unionWith(modulo((boardIndex + 1 - BITBOARD_CONTAINER_COLS)
										,BITBOARD_CONTAINER_SIZE), hashedBoards[3]);
						break;

					case 2:
						//Lower and Right Boards are assigned
						if (hashedBoards[1]) 
							perimeter.unionWith((boardIndex + BITBOARD_CONTAINER_COLS) % 
									BITBOARD_CONTAINER_SIZE, hashedBoards[1]);
						if (hashedBoards[2]) 
							perimeter.unionWith((boardIndex + 1) % BITBOARD_CONTAINER_SIZE,
									hashedBoards[2]);
						if (hashedBoards[3]) 
							perimeter.unionWith((boardIndex + 1 + BITBOARD_CONTAINER_COLS)
									% BITBOARD_CONTAINER_SIZE, hashedBoards[3]);
						break;

					case 3:
						//Lower and Left Boards are assigned
						if (hashedBoards[1]) 
							perimeter.unionWith((boardIndex + BITBOARD_CONTAINER_COLS) % 
									BITBOARD_CONTAINER_SIZE, hashedBoards[1]);
						if (hashedBoards[2]) 
							perimeter.unionWith(modulo((boardIndex - 1),BITBOARD_CONTAINER_SIZE),
									hashedBoards[2]);
						if (hashedBoards[3]) 
							perimeter.unionWith((boardIndex - 1 + BITBOARD_CONTAINER_COLS)
									% BITBOARD_CONTAINER_SIZE, hashedBoards[3]);
						break;
					case 4:
						//Upper and Left Boards are assigned
						if (hashedBoards[1]) 
							perimeter.unionWith(modulo((boardIndex - BITBOARD_CONTAINER_COLS),  
										BITBOARD_CONTAINER_SIZE), hashedBoards[1]);
						if (hashedBoards[2]) 
							perimeter.unionWith(modulo((boardIndex - 1),BITBOARD_CONTAINER_SIZE),
									hashedBoards[2]);
						if (hashedBoards[3]) 
							perimeter.unionWith(modulo((boardIndex - 1 - BITBOARD_CONTAINER_COLS)
										,BITBOARD_CONTAINER_SIZE), hashedBoards[3]);
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

BitboardContainer BitboardContainer::slowGetPerimeter(){
	BitboardContainer perimeter;
	perimeter.initializeTo(*this);
	perimeter.duplicateBoard( hexagonalDirections);	
	perimeter.xorWith(*this);
	return perimeter;
}

void BitboardContainer::unionWith(int boardIndex, unsigned long long board) {
	if (internalBoardCache.find(boardIndex) == internalBoardCache.end()) {
		internalBoards[boardIndex] = board;
		internalBoardCache.insert(boardIndex);
	} else {
		internalBoards[boardIndex] |= board;
	}
}
/*
 * returns a map of all the bits that were set and which board it was set on
 */
unordered_map< int, vector < unsigned long long >> BitboardContainer::split(){
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

list <BitboardContainer> BitboardContainer::splitIntoBitboardContainers() {
	list <BitboardContainer> returnList;
	for (int i: internalBoardCache){
		unsigned long long board = internalBoards[i];
		while(board) {
			unsigned long long leastSignificantBit = board & -board;
			board ^= leastSignificantBit; // remove least significant bit

			BitboardContainer splitted({{i,leastSignificantBit}});
			returnList.push_front(splitted);
		}		
	}
	return returnList;
}

vector <BitboardContainer> BitboardContainer::splitIntoConnectedComponents(){
	vector <BitboardContainer> components;

	BitboardContainer boards;
	boards.initializeTo(*this);

	while (boards.internalBoardCache.size() != 0) {
		int boardIndex = *(boards.internalBoardCache.begin());
		unsigned long long * currentBoard = &boards.internalBoards[boardIndex];

		 while (*currentBoard) {
		    //repeat if there are more points on this board

			//pick some starting node
			unsigned long long leastSignificantBit = *currentBoard & -*currentBoard;
			BitboardContainer returnBitboard({{boardIndex, leastSignificantBit}});

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

void BitboardContainer::print() {
	for (int i: internalBoardCache) {
		cout << i << "\t"  << internalBoards[i] << endl;
	}	

	if (!internalBoardCache.size())  cout << "empty" << endl;
}

int BitboardContainer::hash() {
	return *(internalBoardCache.begin()) + (__builtin_clzll(internalBoards[
				*(internalBoardCache.begin())]) << 8);

}

int BitboardContainer::getRandomBoardIndex() {

	int total = count();
	total = std::rand() % total;
	int boardSelect = 0;
	for (int i : internalBoardCache) {	
		boardSelect += __builtin_popcountll(internalBoards[i]);
		if (boardSelect > total) return i;
	}
	return -1000;
}

BitboardContainer BitboardContainer::getRandom() {
	int randomBoardIndex = getRandomBoardIndex();
	unsigned long long s = internalBoards[randomBoardIndex];
	unsigned long long t;
	//get a random bit from given bitboard
	while( (s & (s - 1))) {
		t = s & dist(e2);
		s = (t) ? t : s;
	}

	return BitboardContainer({{randomBoardIndex, s}});
}
