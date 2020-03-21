#include <vector>
#include <unordered_map>
#include <iostream>

using namespace std;
class BitboardContainer {

  public:
	//TODO: figure out shift direction O(n) sort so you can get rid of log(n) ordered set
	set<int> internalBoardCache;

	int ROW_SHIFT = 8;
	int COLUMN_SHIFT = 1;
	int BITBOARD_HEIGHT = 8;
	int BITBOARD_WIDTH = 8;
	int BITBOARD_CONTAINER_SIZE  = 16;
	int BITBOARD_CONTAINER_ROWS = 4;
	int BITBOARD_CONTAINER_COLS = 4;

	int  boundingBoxes[16][4];
	unsigned long long int internalBoards[16] = {0};

	BitboardContainer() {};
	BitboardContainer(unordered_map<int, unsigned long long>);

	void initialize(unordered_map <int, unsigned long long>);
	void initializeTo(BitboardContainer&);

	void shiftDirection(Direction);
	void shiftDirection(Direction, int);

	void findBoundingBoxes();
	void findBoundingBoxes(int);
	int findConnectedCompBFS(int, int);

	void floodFillStep(BitboardContainer&, BitboardContainer&);
	void floodFill(BitboardContainer);

	void pruneCache();

	void unionWith(BitboardContainer&);
	void intersectionWith(BitboardContainer&);
	void xorWith(BitboardContainer&);

	bool equals(BitboardContainer&);

	unordered_map <int ,unsigned long long> duplicateBoard(vector<Direction>);

	void findAllGates(BitboardContainer&)
	void findGatesContainingPiece(BitboardContainer&, int, int);
	void findAllGates();

	unordered_map <int , vector <unsigned long long> > split();
};




