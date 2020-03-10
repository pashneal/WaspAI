#include <set>
#include <vector>
#include <string>


using namespace std;

class Hive {
	long long int hive[9];
	bool pieceLookupTable[28];
	set<int> pieceLookupSet;
	Piece pieceArray[28];
	static const int CENTER_X = 23;
	static const int CENTER_Y = 11;
  public:

	int turnCounter = 0;

	Hive();
	void insertPiece(char, string, int , int );
	void insertPiece(Piece);
	void print();
	void printDebug();
	bool pieceInHive(char, string);
	bool pieceInHive(int);
	void movePiece(char, string, bool, string, char, string);
	void movePiece(char, string, Direction, char, string);
	void movePiece(int, Direction, int);
	Piece * getPiece(int);
	Piece * getPiece(char, string);
	bool parseCommand(vector<string>);
	static int pieceNumber(char, string);
	static int pieceNumber(Piece);
	set<int> getPieceLookupSet();
	Piece * getPieceArray();
	bool * getPieceLookupTable();
};

