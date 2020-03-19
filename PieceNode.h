#include <list>
#include <unordered_map>

class PieceNode {
		int pieceNumber;
		BitboardContainer bitboard;
	public:	
		list <*PieceNode> neighbors;	

		bool visited = false;
		int visitedNum;
		int lowLink;

		PieceNode();
		void shiftDirection(Direction, numTimes);
		void shiftDirection(Direction);
		void reposition(list&, BitboardContainer&);
		void insert(list&, BitboardContainer&);
}
