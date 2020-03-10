#include <string>
#include <vector>
#include <set>
#include <unordered_map>
#include <iostream>
#include "constants.h"
#include "Piece.h"
#include "Hive.h"
#include "Bitboard.h"
#include "Test.h"

using namespace std;


 int dxdy[6][2] = {{1,1},
				 {2,0},
				 {1,-1},
				 {-1,-1},
				 {-2,0},
				 {-1,1}};

 unordered_map <string, int> pieceNum = {{"Q",0},
									   {"B1",1},
									   {"B2",2},
									   {"G1",3},
									   {"G2",4},
									   {"G3",5},
									   {"A1",6},
									   {"A2",7},
									   {"A3",8},
									   {"S1",9},
									   {"S2",10},
									   {"M",11},
									   {"L",12},
									   {"P",13}
};



 unordered_map < string , vector<Direction> > directions = 
													{{"\\",{Direction::NW,Direction::SE}}, 
													 {"/",{Direction::SW,Direction::NE}},
													 {"-",{Direction::W,Direction::E}}};



// directions are set out like this
//
// 
//     5    ▄▟▙▄    0 
//      ▁▄▟██████▙▄▁  
//     ██grasshoppe██ 
// 4   ██G1        ██    1
//     ██w         ██
//     ██          ██
//      ▔▀▜██████▛▀▔
//    3     ▀▜▛▀      2

bool Test::silent = false;

void Test::pass(bool testPassed, string message){
		if (!testPassed) cout << message << endl;
		if (!silent && testPassed) cout << "Test passed" << endl;
}

void Test::HiveTest::insertPieceTest() {
	cout << "===========Test::Hive::insertPieceTest()==========" << endl;
	Piece testWhiteQueen = Piece('w', "Q", 4, 4);
	Piece testBlackQueen = Piece('b', "Q", 4, 5);
	Piece testWhiteBeetle1 = Piece( 'w', "B1", 5, 5);
	Piece testBlackAnt2 = Piece('b', "A2", 5, 6 );
	Piece testWhiteSpider2 = Piece('w', "S2", 3, 3);

	
	Hive testHive;

	testHive.insertPiece(testWhiteQueen);
	testHive.insertPiece(testBlackQueen);
	testHive.insertPiece(testWhiteBeetle1);
	testHive.insertPiece(testBlackAnt2);
	testHive.insertPiece(testWhiteSpider2);
	testHive.insertPiece('b', "L", 2, 3);
	testHive.insertPiece('w', "G3", 6, 7);

	bool * testHiveLookupTable = testHive.getPieceLookupTable();
	Piece * testHivePieceArray = testHive.getPieceArray();
	

	set<int> correctSet = {0, 1, 5, 10, 14, 21, 26};
	cout << "-------------------------------------------" << endl;
	cout << "correctSetTest" << endl;


	Test::pass(correctSet == testHive.getPieceLookupSet(), "insertPieceTest \n The "
			"pieceSet is incorrect");


	cout << "-------------------------------------------" << endl;
	cout << "correctHiveLookup" << endl;


	for (int i = 0; i < 28; ++i){
		
		cout << i << " ";
		Test::pass((correctSet.find(i) == correctSet.end()) != testHiveLookupTable[i],
				"insertPieceTest \n" 
				"failed to produce correct answer in Hive::pieceLookupTable");
	}
	
	int ii = 0;
	string s[7] = {"Q","B1","G3","S2","Q","A2","L"};
	char c[7] =     {'w','w','w','w','b','b','b'};
	int x[7] = {4,5,6,3,4,5,2};
	int y[7] = {4,5,7,3,5,6,3};
	
	cout << "-------------------------------------------" << endl;
	cout << "correctHiveLookup" << endl;
	for (auto i: correctSet) {		
		cout << ii << " ";
		Test::pass(testHivePieceArray[i].shorthandName == s[ii] && 
				   testHivePieceArray[i].x == x[ii] &&
				   testHivePieceArray[i].y == y[ii] &&
				   testHivePieceArray[i].color == c[ii],
				   "insertPieceTest\nThe hivePieceArray is incorrect");
		ii++;
	}

}

void Test::HiveTest::movePieceTest(){
	cout << "========Test::Hive::movePieceTest()======" << endl;
	int testPiecesSize = 8;
	Piece testPieces[] = {
						Piece('b', "Q", 11, 11),
						Piece('b', "B2", -1, -1),
						Piece('b', "A2", -1, -1),
						Piece('b', "S1", -1, -1),
						Piece('w', "Q", -1, -1),
						Piece('w', "M", -1, -1),
						Piece('w', "L", -1, -1),
						Piece('w', "G2", -1, -1),
						};
	
	unordered_map< string, int> pieceXY[] = {	 
									 {{"x", 11}, {"y", 11}, }, //1
									 {{"x", 12}, {"y", 12}, }, //2
									 {{"x", 14}, {"y", 12}, }, //3
									 {{"x", 10}, {"y", 12}, }, //4
									 {{"x", 9}, {"y", 13}, }, //5
									 {{"x",  8}, {"y", 12}, }, //6
									 {{"x",  7}, {"y", 11}, }, //7
									 {{"x", 12}, {"y", 10}, }, //8
									};
	
	Hive testHive;
	for (auto i = 0; i < testPiecesSize; ++i){
		testHive.insertPiece(testPieces[i]);
	}
	
	testHive.movePiece('b', "B2", Direction::NE, 'b', "Q"); //2
	testHive.movePiece('b', "A2", true, "-", 'b', "B2"); //3
	testHive.movePiece(Hive::pieceNumber('b',"S1"), Direction::W, Hive::pieceNumber('b',"B2")); //4
	testHive.movePiece('w', "Q", false, "\\", 'b', "S1"); //5
	testHive.movePiece('w', "M", Direction::SW, 'w', "Q"); //6
	testHive.movePiece(Hive::pieceNumber('w', "L"), Direction::SW, Hive::pieceNumber('w', "M")); //7
	testHive.movePiece('w', "G2", Direction::SE, 'b', "Q"); //8

	Piece * testHivePieceArray = testHive.getPieceArray();
	for (auto i : testHive.getPieceLookupSet()){
		for (int j = 0 ; j < testPiecesSize; ++j){
			if (i == Hive::pieceNumber(testPieces[j])){
				cout << "pieceNumber " << i << " ";
				Test::pass(testHivePieceArray[i].x == pieceXY[j]["x"] &&
						   testHivePieceArray[i].y == pieceXY[j]["y"], 
						   "movePieceTest()\n"
						   "correct results were not produced");
			}
		}
	}
	
}

void Test::HiveTest::parseCommandTest(){
	Hive testHive;
	cout << "=========Test::Hive::parseCommandTest()========" << endl;

	vector<vector<string>> commands = {
							{"wQ", "."},
							{"bB2", "wQ\\"},
							{ "wA3", "-bB2"},
							{"bB2", "-wQ"},
							{"wA3", "-bB2"},
							{"bG2", "\\wA3"},
							{"wL", "wA3\\"},
							{"wP", "/wA3"}
	};


	vector<Piece> testPieces = {
						Piece('w', "Q", 23, 11),
						Piece('b', "B2", 21, 11),
						Piece('w', "A3", 19, 11),
						Piece('w', "L", 20, 10),
						Piece('b', "G2", 18, 12),
						Piece('w', "P", 18, 10),
						};
	
	for (auto i = commands.begin(); i != commands.end(); ++i){
		testHive.parseCommand(*i);
	}

	Piece * testHivePieceArray = testHive.getPieceArray();
	for (auto piece: testPieces){
		cout << "pieceNumber " << Hive::pieceNumber(piece) << " ";
		Piece comparisonPiece = testHivePieceArray[Hive::pieceNumber(piece)];
		Test::pass(comparisonPiece.x == piece.x && comparisonPiece.y == piece.y,
				   "parseCommandTest\n"
				   "the command was not parsed as expected");
	}
	
}

void Test::BitboardTest::testBitboardBoundings(){

	cout << "===========Test::Bitboard::testBoundingBoxes()==========" << endl;
	unordered_map<int, unsigned long long int> 
		BitboardContainerMap
		={
			{0,9241421619870564865u},
			{1,	220799870910337024u},
			{2,	4627088527065088u},
			{3,2313752099815424u},
			{4,		68719476736u},
			{5,72057594037928448u},
			{6,576460752303440384u},
			{7,4035225266123981312u}
		};

	int expectedBoundingBoxes[][4] = 
	{
		{0,0,7,7},
		{0,1,6,7},
		{4,4,6,6},
		{1,2,6,6},
		{4,4,4,4},
		{0,1,1,7},
		{1,1,6,7},
		{1,1,6,7}
	};

	BitboardContainer testBitboardContainer(BitboardContainerMap);

	for (int i = 0; i < 8; ++i){
		for (int j = 0; j < 4; ++j){

			cout << "board " << i << " coordinate " << j << " ";
			Test::pass(testBitboardContainer.boundingBoxes[i][j] == expectedBoundingBoxes[i][j],
					"expected bounding coordinate is incorrect");
		}
	}

}

void Test::BitboardTest::testShiftDirection(){
	cout << "==========Test::Bitboard::shiftDirection()=======" << endl;
	
	BitboardContainer testBitboardContainer;
	//thanks to http://cinnamonchess.altervista.org/bitboard_calculator/Calc.html for allowing me to create these tests more easily
	vector <unordered_map <int, unsigned long long>> bitboardList =
	{
		{
			{0,2323928329449648128u},
			{1,288795546670598144u}
		},
		
		{
			{3,609009639598005946u},
			{4,826246961152u},
			{5,1610618880u}
		},

		{
			{3,2378943904679710u},
			{4,3227527192u},
			{5,6291480u},
			{7,13402712491054596096u}
		},
		
		{
			{0,1161964714480637952u},
			{1,144397771187815424u}
		},

		{
			{5, 9544151300301102650u}
		},

		{
			{5, 16592972700788225024u},
			{6, 281474976776192u},
			{2, 1},
			{1, 8}
		},
		

	};


	vector <Direction> shiftDirections = {
		Direction::W,
		Direction::S,
		Direction::N,
		Direction::E,
		Direction::NE,
		Direction::SW
	};

	vector <unordered_map <int, unsigned long long>> expectedResults = 
	{
		{
			{0,1161964714480637952u},
			{1,144397771187815424u}
		},

		{
			{3,2378943904679710u},
			{4,3227527192u},
			{5,6291480u},
			{7,13402712491054596096u}
		},

		{
			{3,609009639598005946u},
			{4,826246961152u},
			{5,1610618880u},
			{7,0}
		},

		{
			{0,2323928329449648128u},
			{1,288795546670598144u}
		},

		{ 
			{5, 16592972700788225024u},
			{6, 281474976776192u},
			{2, 1},
			{1, 8}
		},

		{
			{5, 9544151300301102650u}
		}


	};

	for (int i = 0; i < shiftDirections.size(); ++i){
		cout << "Test " << i <<
			" direction: " << shiftDirections[i] << endl ;

		testBitboardContainer = BitboardContainer(bitboardList[i]);
		testBitboardContainer.shiftDirection(shiftDirections[i]);
		for (auto iter: expectedResults[i]){
			int boardIndex = iter.first;
			cout << "\tboard " << boardIndex << " ";

			Test::pass(expectedResults[i][boardIndex] == testBitboardContainer.internalBoards[boardIndex],
					"Expected result incorrect for shiftDirection() ");
			cout << "\t\t Expected board:" <<  expectedResults[i][boardIndex] << endl;
			cout << "\t\t Given board:" << testBitboardContainer.internalBoards[boardIndex] << endl;
		}
	}


}

int main() {
	Test::HiveTest::insertPieceTest();
	Test::HiveTest::movePieceTest();
	Test::HiveTest::parseCommandTest();
	Test::BitboardTest::testBitboardBoundings();
	Test::BitboardTest::testShiftDirection();
	return 0;
}
