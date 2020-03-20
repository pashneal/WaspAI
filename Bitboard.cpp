#include <list>
#include <set>
#include <algorithm>
#include "constants.h"
#include "Bitboard.h"

using namespace std;


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
	//iterate through the map and update board internals
	for (auto keyValueList : predefinedBoards) {
		internalBoardCache.insert(keyValueList.first);
		internalBoards[keyValueList.first] = keyValueList.second;
	}
	findBoundingBoxes();
}

void BitboardContainer::initializeTo(BitboardContainer &other) {

	unordered_map <int , unsigned long long > otherMap;
	for (int i: other.internalBoardCache){
		otherMap[i] = other.internalBoards[i];
	}
	initialize(otherMap);
}
void BitboardContainer::findBoundingBoxes(){
	for (auto boardIndex: internalBoardCache) { 
		findBoundingBoxes(boardIndex);
	}
}
void BitboardContainer::findBoundingBoxes(int boardIndex){
	if (internalBoardCache.find(boardIndex) == internalBoardCache.end()){
		cout << "error 8" << endl;
		throw 8;
	}
	if (internalBoards[boardIndex] == 0){ 
		//delete board from boardCache
		internalBoardCache.erase(boardIndex);
	}

	int minX = 8;
	int minY = 8;
	int maxX = -1;
	int maxY = -1;
	unsigned long long int testBoard = internalBoards[boardIndex];

	for(int x = 0; x < 8; x++){
		for(int y = 0; y < 8; y++){
			int testBit = (testBoard >> (x + y*8)) % 2;
			//find the place of the if it is set
			if (testBit) {
				minX = (minX > x) ? x : minX;
				minY = (minY > y) ? y : minY;
				maxX = (maxX < x) ? x : maxX;
				maxY = (maxY < y) ? y : maxY;
			}
			
		}
	}
	boundingBoxes[boardIndex][0] = minX;
	boundingBoxes[boardIndex][1] = minY;
	boundingBoxes[boardIndex][2] = maxX;
	boundingBoxes[boardIndex][3] = maxY;
}

//This is a messy, buggy, slow function; not meant to be used in the search tree!
void BitboardContainer::shiftDirection(Direction dir){
	unsigned long long tempMask = 0xff00ff00ff00ff;
	unordered_map <int, unsigned long long> tempActiveBoards;

	if (dir == Direction::SE || dir == Direction::SW ||
			dir == Direction::NE || dir == Direction::NW) {

		if ( dir == Direction::SE || dir == Direction::SW) {
			shiftDirection(Direction::S);
			if	(dir == Direction::SE) tempMask = ~tempMask;
		} else {
			shiftDirection(Direction::N);
			if (dir == Direction::NE) tempMask = ~tempMask;
		}

		for (int i: internalBoardCache) {
			//preserve certain rows and then delete them from internal Boards
			tempActiveBoards[i] = internalBoards[i] & tempMask;
			internalBoards[i] &= ~tempMask;
		}

		if (dir == Direction::NE|| dir == Direction::SE)
			shiftDirection(Direction::E);
		else {
			shiftDirection(Direction::W);
		}

		for (auto iter : tempActiveBoards){
			//put the preserved rows back in 
			internalBoards[iter.first] |= iter.second;
		}
		return;
	}
	

	vector <int> activeBoards;
	for (int i : internalBoardCache){
		activeBoards.push_back(i);
	}

	//Sort the activeBoards in descending order if updating the boards in the west/north
	//directions


	if (dir == Direction::E || dir == Direction::S) reverse(activeBoards.begin(), activeBoards.end());


	for (int i = 0; i< activeBoards.size(); ++i){

		unsigned long long westColumnSelector = 0x101010101010101u;
		unsigned long long eastColumnSelector = 0x8080808080808080u;
		unsigned long long southRowSelector =   0xff;
		unsigned long long northRowSelector =   0xff00000000000000u;


		if (dir == Direction::E) {
			eastColumnSelector &= internalBoards[activeBoards[i]];
			if (eastColumnSelector != 0 && !(activeBoards[i] > BITBOARD_CONTAINER_SIZE - 2 ||
						((activeBoards[i] % BITBOARD_CONTAINER_ROWS) == 3))){

				//magic number fix later TODO
				//delete the overflow
				internalBoards[activeBoards[i]] ^= eastColumnSelector;
				eastColumnSelector >>= COLUMN_SHIFT*(BITBOARD_WIDTH - 1);

				internalBoards[activeBoards[i]+ 1] |= eastColumnSelector;
				internalBoardCache.insert(activeBoards[i] + 1);
			}
			internalBoards[activeBoards[i]] <<= COLUMN_SHIFT;

		} else if ( dir == Direction::W) {

			westColumnSelector &= internalBoards[activeBoards[i]];

			if (westColumnSelector != 0 && !(activeBoards[i] < 1 || 
						((activeBoards[i] % BITBOARD_CONTAINER_ROWS) == 0))){

				//delete the overflow
				internalBoards[activeBoards[i]] ^= westColumnSelector;

				westColumnSelector <<= COLUMN_SHIFT*(BITBOARD_WIDTH - 1);

				internalBoards[activeBoards[i] - 1] |= westColumnSelector;

				internalBoardCache.insert(activeBoards[i] - 1);
			}
			internalBoards[activeBoards[i]] >>= COLUMN_SHIFT;

		} else if ( dir == Direction::S) {
			southRowSelector &= internalBoards[activeBoards[i]];
			if (southRowSelector != 0 && 
					!(activeBoards[i] >= (BITBOARD_CONTAINER_SIZE - BITBOARD_CONTAINER_ROWS))){
				//magic number fix later TODO
				//delete the overflow
				internalBoards[activeBoards[i]] ^= southRowSelector;

				southRowSelector <<= ROW_SHIFT*(BITBOARD_HEIGHT - 1);

				internalBoards[activeBoards[i] + BITBOARD_CONTAINER_ROWS] |= southRowSelector;
				internalBoardCache.insert(activeBoards[i] + BITBOARD_CONTAINER_ROWS);
			}
			internalBoards[activeBoards[i]] >>= ROW_SHIFT;

		} else if ( dir == Direction::N) {
			northRowSelector &= internalBoards[activeBoards[i]];
			if (northRowSelector != 0 && (activeBoards[i] > BITBOARD_CONTAINER_ROWS)){
				//magic number fix later TODO
				//delete the overflow
				internalBoards[activeBoards[i]] ^= northRowSelector;
				northRowSelector >>= ROW_SHIFT*(BITBOARD_HEIGHT - 1);

				internalBoards[activeBoards[i] - BITBOARD_CONTAINER_ROWS] |= northRowSelector;
				internalBoardCache.insert(activeBoards[i] - BITBOARD_CONTAINER_ROWS);
			}
			internalBoards[activeBoards[i]] <<= ROW_SHIFT;

		} 	
	}
}

//This is also slow but far less messy!
void BitboardContainer::floodFillStep(BitboardContainer &frontier,  BitboardContainer &visited){
	vector<Direction> traversalList = {
										 Direction::SE,
										 Direction::N,
										 Direction::N,
										 Direction::SW,
										 Direction::SW,
										 Direction::N,
										 Direction::N
										};

	for (Direction dir : traversalList) {
		frontier.shiftDirection(dir);
		visited.unionWith(frontier);
		visited.intersectionWith(*this);
	}
	frontier.shiftDirection(Direction::SE);
	frontier.pruneCache();
}

//TODO test
//TODO optimize because this is so slow it hurts
void BitboardContainer::floodFill(BitboardContainer frontier){
	BitboardContainer visited;
	visited.initializeTo(frontier);	

	do  {
		frontier.initializeTo(visited);
		floodFillStep(frontier, visited);
	} while (!frontier.equals(visited));
}


bool BitboardContainer::equals(BitboardContainer other){
	for (int i = 0; i < BITBOARD_CONTAINER_SIZE; i++){
		if (internalBoards[i] != other.internalBoards[i]) return false;
	}
	return true;
}

void BitboardContainer::pruneCache(){
	list <int> = emptyBoards;
	for (int i: internalBoardCache){
		if (internalBoards[i] == 0){
			internalBoardCache.push_front(i);
		}
	}

	for (int i: emptyBoards) {
		internalBoardCache.erase(i);
	}
}

void BitboardContainer::unionWith( BitboardContainer &other){
	for (auto i: other.internalBoardCache){
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

}

void BitboardContainer::xorWith( BitboardContainer &other) {
	for (auto i: other.internalBoardCache) {
		internalBoards[i] ^= other.internalBoards[i];
		internalBoardCache.insert(i);
	}
}

void BitboardContainer::clear() {
	for (int i : internalBoardCache) {
		internalBoards[i] = 0;
	}
	pruneCache();
}
//TODO optimize
//TODO test
unordered_map<int, unsigned long long> BitboardContainer::duplicateBoard(vector <Direction> dirs){
	BitboardContainer other;
	unordered_map <int , unsigned long long> returnMap;
	other.initializeTo(*this);
	for (int i : other.internalBoardCache){
		returnMap[i] = other.internalBoards[i];
	}
	for (Direction dir: dirs) { 
		other.shiftDirection(dir);
		for (int i: other.internalBoardCache) {
			if (returnMap.find(i) == returnMap.end()) returnMap[i] == 0;
			returnMap[i] |= other.internalBoards[i];
		}
		other.initializeTo(*this); // reset is slow but it prolly works 
	}
}

//Find gate structures from *this board and store is in result
void BitboardContainer::findAllGates(BitboardContainer &result ){
	vector <Direction> dirs = {Direction::NW,Direction::SW,Direction::E,
							   Direction::W,Direction::SE,Direction::NE};

	BitboardContainer hivePerimeter(duplicateBoard(dirs));

	hivePerimeter.xorWith(*this);
}

//find gate structures from *this board and store it in result, 
//only search around given areas
//
void BitboardContainer::findGatesContainingPiece(BitboardContainer &result) {

}
