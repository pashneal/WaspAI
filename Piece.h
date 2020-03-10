#include <string>
#include <unordered_map>
#include <vector>

using namespace std;



class Piece {
	
	int boardNumber;
	int bitboard;

 public:
	
	char color;
	string name;
	string shorthandName;
	int x;
	int y;
	Piece() {};
	Piece(char, string , int , int );
	
};
