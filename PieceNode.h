#include <list>
#include <unordered_map>

using namespace std;

class PieceNode {
		int pieceNumber;
		BitboardContainer bitboard;
	public:	
		list <PieceNode*> neighbors;	

		bool visited = false;
		int visitedNum;
		int lowLink;
		bool isEmpty;
		PieceNode * parent;

		PieceNode();
		PieceNode(int, BitboardContainer);
		void shiftDirection(Direction, int);
		void shiftDirection(Direction);
		void reposition(list<PieceNode*>&, BitboardContainer&);
		void insert(list<PieceNode*>&, BitboardContainer&);
};
