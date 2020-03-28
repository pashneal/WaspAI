#include <list>
#include <set>
#include <algorithm>
#include "constants.h"
#include "Bitboard.h"

using namespace std;

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


unordered_map <Direction, vector<int>> parameters  = {
	{Direction::E, {1, 1, 1}},
	{Direction::W, {0, 1, -1}},
	{Direction::S, {1, 8, 4}},
	{Direction::N, {0, 8, -4}}
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

	unordered_map <int , unsigned long long > otherMap;
	clear();
	for (int i: other.internalBoardCache){
		otherMap[i] = other.internalBoards[i];
	}

	initialize(otherMap);
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


	int overflowAmount = numTimes % 8;
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
		//TODO: shift direction may be able to overflow
		if (numTimes >= BITBOARD_HEIGHT) {
			internalBoardCache.erase(initialBoardIndex);
		}

		overflowHigh = overflowHighMask & internalBoards[initialBoardIndex];
		overflowLow = overflowLowMask & internalBoards[initialBoardIndex];

		overflowLow = adjustOverflowMask(dir, overflowAmount, true, overflowLow);
		overflowHigh = adjustOverflowMask(dir, overflowAmount, false, overflowHigh);

		if (newLowBoardIndex < 16 && newLowBoardIndex >= 0) {

			internalBoards[newLowBoardIndex] = 0;
			internalBoards[newLowBoardIndex] |= overflowLow;
			internalBoardCache.insert(newLowBoardIndex);
		}

		if (newHighBoardIndex < 16 && newHighBoardIndex >= 0) {

			if (internalBoardCache.find(newHighBoardIndex) == internalBoardCache.end()) 
				internalBoards[newHighBoardIndex] = 0;

			internalBoards[newHighBoardIndex] |= overflowHigh;
			internalBoardCache.insert(newHighBoardIndex);
		}


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

void BitboardContainer::shiftDirection(Direction dir) {
	shiftDirection(dir, 1);
	//TODO: optimize by storing possible overflow mask configs
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

bool BitboardContainer::containsAny(BitboardContainer& other) {
	bool any = 0;	
	for (int i: other.internalBoardCache) { 
		if (internalBoardCache.find(i) != internalBoardCache.end())
			any |= (internalBoards[i] & other.internalBoards[i]);
	}
	return any;
}

void BitboardContainer::clear() {
	internalBoardCache.clear();
}
//TODO optimize
//TODO test
void BitboardContainer::duplicateBoard(vector <Direction> dirs){


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


BitboardContainer  BitboardContainer::getPerimeter() {
	BitboardContainer perimeter;
	
	vector <Direction> directions = {Direction::E,Direction::SE,Direction::NE,
										Direction::NW,Direction::SW,Direction::W};

	perimeter.initializeTo(*this);
	perimeter.duplicateBoard( directions);	
	perimeter.xorWith(*this);

	return perimeter;
}

/*
 * returns a map of all the bits that were set and which board it was set on
 */
unordered_map< int, vector < unsigned long long >> BitboardContainer::split(){

	unordered_map< int, vector <unsigned long long >> returnMap;

	for (int i: internalBoardCache){
		unsigned long long board = internalBoards[i];
		while (board) {
			unsigned long long leastSignificantBit = board & -board;
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
