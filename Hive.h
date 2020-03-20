#include <set>
#include <vector>
#include <string>

using namespace std;

class Hive {
	static const int CENTER_X = 23;
	static const int CENTER_Y = 11;
	//TODO make tests and center_xy not dependent on coords


	bool pieceLookupTable[28];
	set<int> pieceLookupSet;
	Piece pieceArray[28];

	int BitBoardShiftY = 0;
	int BitBoardShiftX = 0;

	BitboardContainer allPieces;
	BitboardContainer whitePieces;
	BitboardContainer blackPieces;
	BitboardContainer ants;
	BitboardContainer beetles;
	BitboardContainer spiders;
	BitboardContainer ladybugs;
	BitboardContainer queens;
	BitboardContainer mosquitoes;
	BitboardContainer pillbugs;
	BitboardContainer grasshoppers;
	BitboardContainer gates;
	BitboardContainer doors;
	BitboardContainer rings;
	BitboardContainer firstPieces;
	BitboardContainer secondPieces;
	BitboardContainer thirdPieces;

	list <PieceNode*> pieceNodes; 
	
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
	vector <PieceNode*> getArticulationNodes();
	void depthFirstSearch();
	void traverseNodes(PieceNode&, int&);
	void updateArticulationFramework(vector <PieceNode*>&);
	void assignLowLink();
	void updateLowLink();
};

