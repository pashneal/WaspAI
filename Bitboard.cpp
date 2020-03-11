#include <algorithm>
#include "constants.h"
#include "Bitboard.h"

using namespace std;


BitboardContainer::BitboardContainer(){
	//put constructor stuff here
}

BitboardContainer::BitboardContainer(unordered_map<int, unsigned long long int> predefinedBoards){

	for (int i = 0; i < 16; i++) {
		internalBoards[i] = 0;
	}
	//iterate through the map and update board internals
	for (auto keyValueList : predefinedBoards) {
		internalBoardCache.insert(keyValueList.first);
		internalBoards[keyValueList.first] = keyValueList.second;
	}
	findBoundingBoxes();

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

void BitboardContainer::shiftDirection(Direction dir){
	
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
				!(activeBoards[i] >= (BITBOARD_CONTAINER_SIZE - BITBOARD_CONTAINER_ROWS - 1))){
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

		} else if ( dir == Direction::SE) {
			shiftDirection(Direction::S);
			shiftDirection(Direction::E);
			return;

		} else if ( dir == Direction::NE) {
			shiftDirection(Direction::N);
			shiftDirection(Direction::E);
			return;

		} else if ( dir == Direction::NW) {
			shiftDirection(Direction::N);
			shiftDirection(Direction::W);
			return;

		} else if ( dir == Direction::SW){
			shiftDirection(Direction::S);
			shiftDirection(Direction::W);
			return;
		}
	}
}


