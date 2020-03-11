#include <set>
#include <vector>
#include <unordered_map>
#include <iostream>

using namespace std;
class BitboardContainer {

  public:
	set<int> internalBoardCache;

	int ROW_SHIFT = 8;
	int COLUMN_SHIFT = 1;
	int BITBOARD_HEIGHT = 8;
	int BITBOARD_WIDTH = 8;
	int BITBOARD_CONTAINER_SIZE  = 16;
	int BITBOARD_CONTAINER_ROWS = 4;
	int BITBOARD_CONTAINER_COLS = 4;

	int  boundingBoxes[16][4];
	unsigned long long int internalBoards[16];

	BitboardContainer();
	BitboardContainer(unordered_map<int, unsigned long long int>);

	void shiftDirection(Direction);
	void findBoundingBoxes();
	void findBoundingBoxes(int);
	int findConnectedCompBFS(int, int);
	void floodFillStep(BitboardContainer, BitboardContainer);
	void pruneCache();
	void unionWith(BitboardContainer);
	void andWith(BitboardContainer);
};


