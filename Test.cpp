#include <stdlib.h>
#include <time.h>
#include <string>
#include <vector>
#include <set>
#include <unordered_map>
#include <iostream>
#include "constants.h"
#include "Bitboard.h"
#include "MoveGenerator.h"
#include "Piece.h"
#include "PieceNode.h"
#include "ProblemNode.h"
#include "Hive.h"
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
		if (!testPassed) cout << "\033[1;31m" << message << "\033[0m" << endl;
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
			{5, 2258440288213000u}
		},

		{
			{5, 579849679608481792u}
		},

		{
			{5,2258444583180296}
		}

	};


	vector <Direction> shiftDirections = {
		Direction::W,
		Direction::S,
		Direction::N,
		Direction::E,
		Direction::NE,
		Direction::SW,
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
			{5, 579849679608481792u}
		},

		{
			{5, 2258440288213000u}
		},

		{
			{5, 4423901974552},{9,288230376151711744},{4, 2147483648}
		}


	};

	for (unsigned long long i = 0; i < shiftDirections.size(); ++i){
		cout << "Test " << i <<
			" direction: " << shiftDirections[i] << endl ;

		testBitboardContainer.initialize(bitboardList[i]);
		testBitboardContainer.shiftDirection(shiftDirections[i]);
		for (auto iter: expectedResults[i]){
			int boardIndex = iter.first;
			cout << "\tboard " << boardIndex << " ";

			Test::pass(expectedResults[i][boardIndex] == testBitboardContainer.internalBoards[boardIndex],
					"Expected result incorrect for shiftDirection() ");
			//cout << "\t\t Expected board:" <<  expectedResults[i][boardIndex] << endl;
			//cout << "\t\t Given board:" << testBitboardContainer.internalBoards[boardIndex] << endl;
		}
	}

	unordered_map <int, unsigned long long> test;
	test[5] = 1;
	testBitboardContainer.initialize(test);
	testBitboardContainer.shiftDirection(Direction::NE);
	testBitboardContainer.shiftDirection(Direction::W);
	testBitboardContainer.shiftDirection(Direction::SW);
	testBitboardContainer.shiftDirection(Direction::SE);
	testBitboardContainer.shiftDirection(Direction::E);
	testBitboardContainer.shiftDirection(Direction::NW);
	cout << "Test " << shiftDirections.size() << ": ";
	Test::pass(testBitboardContainer.internalBoards[5] == 1, "result incorrect");


	cout << "Test " << shiftDirections.size() + 1 << ": ";

	BitboardContainer testProblemNodes;
	testProblemNodes.initialize({{12, 524800u}});
	testProblemNodes.shiftDirection(Direction::N, 15);
	testProblemNodes.shiftDirection(Direction::E, 7);
	testProblemNodes.convertToHexRepresentation(Direction::NE, 15);
	BitboardContainer result({{5,1026u}});
	Test::pass(result.equals(testProblemNodes), " result incorrect");
	
}

void Test::BitboardTest::testXorWith() {
	cout << "===================TestXORWith===================" << endl;
	vector <unordered_map <int, unsigned long long>> bitboardInitialList =
	{
		{{0,262519634108789134u}},
		{{1,0x70b46ca6fc5b5u},
		 {2,0x9120002a48u}}
	};

	vector <unordered_map <int, unsigned long long >> bitboardCompList = {
		{{0,2026862587613u}},
		{{3,2026862587613u},
		 {2,202686u}}
	};

	vector <unordered_map <int, unsigned long long>> expectedResults  =
	{
		{{0, 262521460180000595u}},
		{{1,0x70b46ca6fc5b5u},
		{2,623307341302u},
		{3,2026862587613u}}
	};

	BitboardContainer test, compare;
	for (unsigned long long i = 0; i < expectedResults.size(); ++i){
		cout << "Test " << i << endl;
		
		test.initialize(bitboardInitialList[i]);
		compare.initialize(bitboardCompList[i]);
		test.xorWith(compare);
		for (auto j : expectedResults[i]){
				cout << "\t Board: " << j.first << " ";

				Test::pass(test.internalBoards[j.first] == j.second, 
						"failed to get correct result for xorWith()");
		}
	}
}

void Test::BitboardTest::testIntersectionWith() {
	cout << "===================TestANDWith===================" << endl;
	vector <unordered_map <int, unsigned long long>> bitboardInitialList =
	{
		{{0,262519634108789134u}},
		{{1,0x70b46ca6fc5b5u},
		 {2,0x9120002a48u}}
	};

	vector <unordered_map <int, unsigned long long >> bitboardCompList = {
		{{0,2026862587613u}},
		{{3,2026862587613u},
		 {2,202686u}}
	};

	vector <unordered_map <int, unsigned long long>> expectedResults  =
	{
		{{0, 100395688076u}},
		{{2,520u}}
	};


	BitboardContainer test, compare;
	for (unsigned long long i = 0; i < expectedResults.size(); ++i){
		cout << "Test " << i << endl;

		test.initialize(bitboardInitialList[i]);
		compare.initialize(bitboardCompList[i]);
		test.intersectionWith(compare);
		BitboardContainer results(expectedResults[i]);

		Test::pass(test == results, 
				"failed to get correct result for intersectionWith()");
	}
}

void Test::BitboardTest::testUnionWith() {
	cout << "===================TestORWith===================" << endl;
	vector <unordered_map <int, unsigned long long>> bitboardInitialList =
	{
		{{0,262519634108789134u}},
		{{1,0x70b46ca6fc5b5u},
		 {2,0x9120002a48u}}
	};

	vector <unordered_map <int, unsigned long long >> bitboardCompList = {
		{{0,2026862587613u}},
		{{3,2026862587613u},
		 {2,202686u}}
	};

	vector <unordered_map <int, unsigned long long>> expectedResults  =
	{
		{{0, 262521560575688671u}},
		{{2,623307341822u}, {1,0x70b46ca6fc5b5u}, {3,2026862587613u}}
	};


	BitboardContainer test, compare;
	for (unsigned long long i = 0; i < expectedResults.size(); ++i){
		cout << "Test " << i << endl;
		
		test.initialize(bitboardInitialList[i]);
		compare.initialize(bitboardCompList[i]);
		test.unionWith(compare);
		for (auto j : expectedResults[i]){
				cout << "\t Board: " << j.first << " ";

				Test::pass(test.internalBoards[j.first] == j.second, 
						"failed to get correct result for UnionWith()");
		}

	}
}

void Test::BitboardTest::testContainsAny() {
	cout << "===================TestContainsAny===================" << endl;
	vector <unordered_map <int, unsigned long long>> bitboardInitialList =
	{
		{{0,262519634108789134u}},
		{{1,0x70b46ca6fc5b5u},
			{2,0x9120002a48u}}
	};

	vector <unordered_map <int, unsigned long long >> bitboardCompList = {
		{{0,2026862587613u}},
		{{5,2026862587613u},
			{3,202686u}}
	};

	vector <bool> expectedResults  =
	{1,0};


	BitboardContainer test, compare;
	for (unsigned long long i = 0; i < expectedResults.size(); ++i){
		cout << "Test " << i << endl;

		test.initialize(bitboardInitialList[i]);
		compare.initialize(bitboardCompList[i]);
		bool v = test.containsAny(compare);
		cout << "\t Board: " << expectedResults[i] << " ";

		Test::pass(v == expectedResults[i], 
				"failed to get correct result for containsAny()");

	}
}

void Test::BitboardTest::testFloodFillStep() {
		
	cout << "===================TestFloodFillStep===================" << endl;

	vector <unordered_map <int, unsigned long long>> bitboardTraversableList = {
		{{0,8831493474304u}},
		{{0,49344u}, {1,1}, {4,13835058055282163712u}},
		{{0,49344u}, {1,1}, {4,13835058055282163712u}}
	};
	vector <unordered_map <int, unsigned long long>> bitboardFrontierList =
	{
		{{0,134217728u}},
		{{0,128}},
		{{0,49216u}, {1,1}, {4,13835058055282163712u}}
	};

	vector <unordered_map <int, unsigned long long >> bitboardVisitedList = {
		{{0,0}},
		{{0,0}},
		{{0,128}}
	};


	vector <unordered_map <int, unsigned long long >> expectedVisitedList = {
		{{0,134217728u}},
		{{0,128}},
		{{0,49344u}, {1,1}, {4,13835058055282163712u}}
	};

	vector <unordered_map <int, unsigned long long >> expectedFrontierList = {
		{{0,34695806976u}},
		{{0,49216u}, {1,1}, {4,13835058055282163712u}},
		{{}}
	};
	BitboardContainer traversable, frontier, visited;
	for (unsigned long long i = 0; i < expectedFrontierList.size(); ++i){
		cout << "Test " << i << endl;
		
		traversable.initialize(bitboardTraversableList[i]);
		frontier.initialize(bitboardFrontierList[i]);
		visited.initialize(bitboardVisitedList[i]);

		traversable.floodFillStep(frontier, visited);

		BitboardContainer compVisited(expectedVisitedList[i]);
		BitboardContainer compFrontier(expectedFrontierList[i]);

		visited.pruneCache();
		frontier.pruneCache();
		compVisited.pruneCache();
		compFrontier.pruneCache();
		
		cout << "\t" ;
		Test::pass(compVisited.equals(visited), 
				"visited board is incorrect for floodFillStep");
		cout << "\t" ;
		Test::pass(compFrontier.equals(frontier), 
				"frontier board is incorrect for floodFillStep");
	}
}

void Test::BitboardTest::testFloodFill() {
	cout << "===================TestFloodFill===================" << endl;

	//I'm too lazy to add more tests but there is space if future me wants to

	vector <unordered_map <int, unsigned long long>> bitboardTraversableList = {
		{{0,144680354303193280u}, {1,396553u}, {4,4629806107761573888u}}
	};
	vector <unordered_map <int, unsigned long long>> bitboardFrontierList =
	{
		{{0,144115188075855872}}
	};

	vector <unordered_map <int, unsigned long long >> expectedFrontierList = {
		{{0,144680354303193280u}, {1,396553u}, {4,4629806107761573888u}}
	};

	BitboardContainer traversable, frontier ;
	for (unsigned long long i = 0; i < expectedFrontierList.size(); ++i){
		cout << "Test " << i << endl;
		traversable.initialize(bitboardTraversableList[i]);
		frontier.initialize(bitboardFrontierList[i]);
		traversable.floodFill(frontier);
		BitboardContainer compFrontier(expectedFrontierList[i]);
		frontier.pruneCache();
		compFrontier.pruneCache();
		cout << "\t" ;
		Test::pass(compFrontier.equals(frontier), 
				"frontier board is incorrect for floodFill");
	}
}

void Test::BitboardTest::testSplit() {
	srand(time(NULL));
	vector <unsigned long long> randomNums;

	BitboardContainer testBitboard;
	for (int i = 0; i < 16; i++) {
		randomNums.push_back(rand() % (1 << 30));
		testBitboard.setBoard(i, randomNums[i]);
	}

	cout << "====================TestSplit====================" << endl;

	for (auto map: testBitboard.split()) {
		unsigned long long test  = 0;
		cout << map.first << " ";
		for (unsigned long long i: map.second) {
			test |= i;
		}
		Test::pass(test == randomNums[map.first], 
				" did not get expected result for split()");
	}
}

void Test::BitboardTest::testSplitIntoConnectedComponents() {
	cout << "===================TestSplitIntoConnectedComponents===================" << endl;

	//I'm too lazy to add more tests but there is space if future me wants to

	unordered_map <int, unsigned long long> bitboardTraversable 
		{{0,144680354236084224u}, {1,265481u},
		 {4,4629806107769962496u}, {5, 216172782113849344u}};

	vector <unordered_map <int, unsigned long long >> expectedComponentsList= {
		{{0,144680354232401920u}},
		{{0,3682304u}},
		{{1,265224u}},
		{{1,257}, {5,216172782113783808u}},
		{{4,8388608u}, {5,65536u}},
		{{4, 4629806107761573888u}}
	};

	BitboardContainer traversable(bitboardTraversable);
	int testNum = 0;

	for (BitboardContainer result: traversable.splitIntoConnectedComponents()) {
		cout << "Test " << testNum << endl << "\t";
		bool flag = 0;
		for (BitboardContainer test: expectedComponentsList){ 
			flag |= test.equals(result);
		}
		Test::pass(flag, "expectedComponentsList test failed");
		testNum++;
	}
}

void Test::ProblemNodeContainerTest::testFindAllProblemNodes(){
	cout << "===================FindAllProblemNodes===================" << endl;
	//TODO: bug when the the piecesi shiftDirection too far horizontally
	vector <unordered_map <int, unsigned long long>> gatesTest = {
		{{0,17632004u}},
		{{2,4484114285830u}},
		{{4,36099441180057792u}, {5, 846645512438272u}},
		{{12, 51708299278u}},
		{{15, 2361380u}},
		{{9, 1116741894656u}},
		{{6,549755813888u}, {7,16777216u}},
		{{6, 806367232u}},
		{{6, 2147516416u}, {7, 16908544u}}
	};

	vector <unordered_map <int, unsigned long long >> expected= {
		{{0, 33686536u}},
		{{2, 8830520132096u}},
		{{4, 141287244169216u}, {5, 1108135248128u}},
		{{12,1792u}},
		{{15,1586712u}},
		{{9,2211908550912u}},
		{{6,2147483648u}, {7, 4294967296u}},
		{{6,6291456}},
		{{6, 8388608u}, {7, 65536u}}
	};

	for (unsigned int i = 0; i < expected.size(); i++ ){
		cout << "Test" << i << ": ";
		BitboardContainer gateTest(gatesTest[i]);
		BitboardContainer expectedProblemNodes(expected[i]);
		ProblemNodeContainer test(&gateTest);
	
		test.findAllProblemNodes();
		Test::pass(test.visibleProblemNodes == expectedProblemNodes,
					"incorrect board returned for problemNodes");
		/*cout << "given Board: " ;
		for (int i : test.visibleProblemNodes.internalBoardCache) {
			cout << "\n " << i << " " << test.visibleProblemNodes.internalBoards[i];
		}
		cout << endl;*/
	}
}


void Test::ProblemNodeContainerTest::testRemovePiece(bool noMessage){
	//This test will not work if FindAllProblemNodes() does not work
	cout << "===================TestRemovePiece===================" << endl;
	vector <unordered_map <int, unsigned long long>> gatesTest = {
		{{0,17632004u}},
		{{2,4484114285830u}},
		{{4,36099441180057792u}, {5, 846645512438272u}},
		{{12, 51708299278u}},
		{{15, 2361380u}},
		{{9, 1116741894656u}},
		{{6,549755813888u}, {7,16777216}},
		{{9,50660096u} , {8,3225468928u}}
	};

	vector <unordered_map <int, unsigned long long >> removePiece = {
		{{0, 4}},
		{{2, 1024}},
		{{5, 16777216u}},
		{{12,17179869184u}},
		{{15, 2048u}},
		{{9, 1099511627776u}},
		{{7,16777216}},
		{{9,65536}}
	};

	vector <unordered_map <int, unsigned long long >> expected= {
		{{0, 33685504u}},
		{{2, 8830520131584u}},
		{{4, 141289391652864u}, {5, 1108118470912u}},
		{{12, 17246979840u}},
		{{15, 13824u}},
		{{9, 393472u}},
		{{}},
		{{8,8388608u}, {9,196608u}}
		
	};

	for (unsigned int i = 0; i < gatesTest.size(); i++) {
		cout << "Test " << i << ": ";
		
		BitboardContainer initialBoard(gatesTest[i]);
		BitboardContainer removePieceBoard(removePiece[i]);
		ProblemNodeContainer test(&initialBoard);
		BitboardContainer expectedProblemNodes(expected[i]);

		test.findAllProblemNodes();
		test.removePiece(removePieceBoard);

		Test::pass(test.visibleProblemNodes == expectedProblemNodes,
					"incorrect board returned for problemNodes");

		if (!noMessage) {
			cout << "given Board: " ;
			for (int i : test.visibleProblemNodes.internalBoardCache) {
				cout << "\n " << i << " " << test.visibleProblemNodes.internalBoards[i];
			}
			
			cout << endl;

			cout << "expected Board: " ;
			for (int i : expectedProblemNodes.internalBoardCache) {
				cout << "\n " << i << " " << expectedProblemNodes.internalBoards[i];
			}

			cout << endl <<endl ;
		}
	}
}

void Test::MoveGeneratorTest::testQueenMoves() {
	cout << "===================TestQueenMoves===================" << endl;
	PieceName name = PieceName::QUEEN;
	bool silenced =true;

	vector <unordered_map <int, unsigned long long>> test = {
		{{9, 2258422653255680u}},
		{{10, 103550550016u}},
		{{13, 103550025728u}},
		{{5, 827595993088u}, {6, 16908544u}},
		{{4,1621106688u}}
	};

	vector <unordered_map<int, unsigned long long>> piece = {
		{{9,2251799813685248u}},
		{{10, 134217728u}},
		{{13, 134217728u}},
		{{5 , 8388608u}},
		{{4, 8388608u}}
	};

	vector <unordered_map <int, unsigned long long >> expected = {
		{{9, 1134695999864832u}},
		{{}},
		{{}},
		{{5,1073758208u}},
		{{4, 2151677952u}}
	};


	for (unsigned long long i = 0; i < expected.size(); i++ ) {
		cout << "Test " << i << ": ";
		BitboardContainer testBoard(test[i]);
		BitboardContainer expectedBoard(expected[i]);
		BitboardContainer pieceBoard(piece[i]);

		ProblemNodeContainer problemNodeCont(&testBoard);
		problemNodeCont.findAllProblemNodes();

		MoveGenerator moveGen(&testBoard, &problemNodeCont);
		moveGen.setGeneratingName(&name);
		moveGen.setGeneratingPieceBoard(&pieceBoard, false);
	
		BitboardContainer moves = moveGen.getMoves();
		Test::pass( moves == expectedBoard, 
				"incorrect moves outputted for move generation");

		if (!silenced) {
			for (int boardIndex: moves.internalBoardCache) {
				cout << boardIndex << " ";
				cout << moves.internalBoards[boardIndex] << endl;
			} 
			if (moves.internalBoardCache.size() == 0) cout << "given boards empty " << endl;
		}
	}
}

void Test::MoveGeneratorTest::testSpiderMoves() {
	cout << "===================TestSpiderMoves===================" << endl;
	PieceName name = PieceName::SPIDER;
	bool silenced =true;

	vector <unordered_map <int, unsigned long long>> test = {
		{{9, 68921589760u}},
		{{10, 103550550016u}},
		{{13, 103550025728u}},
		{{5, 827595993088u}, {6, 16908544u}},
		{{4,1621106688u}},
		//TODO: stop being lazy with this test 
		{{14, 211381651554304u}, {15, 4479648768u}},
		{{3,1006910464u}}
	};

	vector <unordered_map<int, unsigned long long>> piece = {
		{{9,68719476736u}},
		{{10, 134217728u}},
		{{13, 134217728u}},
		{{5 , 8388608u}},
		{{4, 8388608u}},
		{{15,131072u}},
		{{3, 536870912u}}
	};

	vector <unordered_map <int, unsigned long long >> expected = {
		{{9, 33556480u}},
		{{}},
		{{}},
		{{5, 1073758240u}, {6, 1}},
		{{4, 274877907008u}},
		{{14, 550829555712u} , {15, 51608291333u}},
		{{3, 34359754752u}}
	};


	for (unsigned long long i = 0; i < expected.size(); i++ ) {
		if (!silenced) cout << endl;
		cout << "Test " << i << ": ";
		BitboardContainer testBoard(test[i]);
		BitboardContainer expectedBoard(expected[i]);
		BitboardContainer pieceBoard(piece[i]);

		ProblemNodeContainer problemNodeCont(&testBoard);
		problemNodeCont.findAllProblemNodes();

		MoveGenerator moveGen(&testBoard, &problemNodeCont);
		moveGen.setGeneratingName(&name);
		moveGen.setGeneratingPieceBoard(&pieceBoard, false);
	
		BitboardContainer moves = moveGen.getMoves();
		Test::pass( moves == expectedBoard, 
				"incorrect moves outputted for move generation");

		if (!silenced) {
			moves.print();
		}
	}
}

void Test::MoveGeneratorTest::testBeetleMoves() {
	cout << "===================TestBeetleMoves===================" << endl;
	PieceName name = PieceName::BEETLE;
	bool silenced = false;

	vector <unordered_map <int, unsigned long long>> test = {
		{{9, 2258422653255680u}},
		{{10, 103550550016u}},
		{{13, 103550025728u}},
		{{5, 827595993088u}, {6, 16908544u}},
		{{4,1621106688u}}
	};

	vector <unordered_map<int, unsigned long long>> piece = {
		{{9,2251799813685248u}},
		{{10, 134217728u}},
		{{13, 134217728u}},
		{{5 , 8388608u}},
		{{4, 8388608u}}
	};

	vector <unordered_map <int, unsigned long long >> expected = {
		{{9, 1139094046375936u}},
		{{10,103416332288u}},
		{{13,103415808000u}},
		{{5, 3221274624u}},
		{{4, 3225419776u}}
	};


	for (unsigned long long i = 0; i < expected.size(); i++ ) {
		cout << "Test " << i << ": ";
		BitboardContainer testBoard(test[i]);
		BitboardContainer expectedBoard(expected[i]);
		BitboardContainer pieceBoard(piece[i]);

		ProblemNodeContainer problemNodeCont(&testBoard);
		problemNodeCont.findAllProblemNodes();

		MoveGenerator moveGen(&testBoard, &problemNodeCont);
		moveGen.setGeneratingName(&name);
		moveGen.setGeneratingPieceBoard(&pieceBoard, false);
	
		BitboardContainer moves = moveGen.getMoves();
		Test::pass( moves == expectedBoard, 
				"incorrect moves outputted for move generation");

		if (!silenced) {
			for (int boardIndex: moves.internalBoardCache) {
				cout << boardIndex << " ";
				cout << moves.internalBoards[boardIndex] << endl;
			} 
			if (moves.internalBoardCache.size() == 0) cout << "given boards empty " << endl;
		}
	}
}

void Test::MoveGeneratorTest::testGrasshopperMoves() {
	cout << "===================TestGrasshopperMoves===================" << endl;
	PieceName name = PieceName::GRASSHOPPER;
	bool silenced = true;

	vector <unordered_map <int, unsigned long long>> test = {
		{{4, 10155194288446468u}},
		{{5, 203164672u}}
	};

	vector <unordered_map<int, unsigned long long>> piece = {
		{{4 , 134217728u }},
		{{5,  524288u}}
	};

	vector <unordered_map <int, unsigned long long >> expected = {
		{{4, 2449958197323104288u}},
		{{5, 85901574164u}}
	};


	for (unsigned long long i = 0; i < expected.size(); i++ ) {
		cout << "Test " << i << ": ";
		BitboardContainer testBoard(test[i]);
		BitboardContainer expectedBoard(expected[i]);
		BitboardContainer pieceBoard(piece[i]);

		ProblemNodeContainer problemNodeCont(&testBoard);
		problemNodeCont.findAllProblemNodes();

		MoveGenerator moveGen(&testBoard, &problemNodeCont);
		moveGen.setGeneratingName(&name);
		moveGen.setGeneratingPieceBoard(&pieceBoard, false);
	
		BitboardContainer moves = moveGen.getMoves();
		Test::pass( moves == expectedBoard, 
				"incorrect moves outputted for move generation");

		if (!silenced) {
			for (int boardIndex: moves.internalBoardCache) {
				cout << boardIndex << " ";
				cout << moves.internalBoards[boardIndex] << endl;
			} 
			if (moves.internalBoardCache.size() == 0) cout << "given boards empty " << endl;
		}
	}
}

void Test::MoveGeneratorTest::testLadybugMoves() {
	cout << "===================TestLadybugMoves===================" << endl;
	PieceName name = PieceName::LADYBUG;
	bool silenced = false;

	vector <unordered_map <int, unsigned long long>> test = {
		{{9, 68921589760u}},
		{{10, 103550550016u}},
		{{13, 103550025728u}},
		{{5, 827595993088u}, {6, 16908544u}},
		{{4,1621106688u}},
		{{3,1006910464u}}
	};

	vector <unordered_map<int, unsigned long long>> piece = {
		{{9,68719476736u}},
		{{10, 134217728u}},
		{{13, 134217728u}},
		{{5 , 8388608u}},
		{{4, 8388608u}},
		{{3, 536870912u}}
	};

	vector <unordered_map <int, unsigned long long >> expected = {
		{{9, 51574213632u}},
		{{10, 30941517192192u}},
		{{13, 30941517715456u}},
		{{5, 211107306274816u}, {6, 12918522371u}},
		{{4, 412589490176u}},
		{{3, 103080787968u}}
	};


	for (unsigned long long i = 0; i < expected.size(); i++ ) {
		if (!silenced) cout << endl;
		cout << "Test " << i << ": ";
		BitboardContainer testBoard(test[i]);
		BitboardContainer expectedBoard(expected[i]);
		BitboardContainer pieceBoard(piece[i]);

		ProblemNodeContainer problemNodeCont(&testBoard);
		problemNodeCont.findAllProblemNodes();

		MoveGenerator moveGen(&testBoard, &problemNodeCont);
		moveGen.setGeneratingName(&name);
		moveGen.setGeneratingPieceBoard(&pieceBoard, false);
	
		BitboardContainer moves = moveGen.getMoves();
		Test::pass( moves == expectedBoard, 
				"incorrect moves outputted for move generation");

		if (!silenced) {
			moves.print();
		}
	}
}

void Test::MoveGeneratorTest::testAntMoves() {
	cout << "===================TestAntMoves===================" << endl;
	PieceName name = PieceName::ANT;
	bool silenced = false;

	vector <unordered_map <int, unsigned long long>> test = {
		{{9, 68921589760u}},
		{{10, 103550550016u}},
		{{13, 103550025728u}},
		{{5, 827595993088u}, {6, 16908544u}},
		{{4,1621106688u}},
		{{3,1006910464u}}
	};

	vector <unordered_map<int, unsigned long long>> piece = {
		{{9,68719476736u}},
		{{10, 134217728u}},
		{{13, 134217728u}},
		{{5 , 8388608u}},
		{{4, 8388608u}},
		{{3, 536870912u}}
	};

	vector <unordered_map <int, unsigned long long >> expected = {
		{{9, 51842780672u}},
		{{}},
		{{}},
		{{5, 246497975748848}, {6, 12918718979u}},
		{{4, 964493856864u}},
		{{3, 257738031740u}}
	};


	for (unsigned long long i = 0; i < expected.size(); i++ ) {
		if (!silenced) cout << endl;
		cout << "Test " << i << ": ";
		BitboardContainer testBoard(test[i]);
		BitboardContainer expectedBoard(expected[i]);
		BitboardContainer pieceBoard(piece[i]);

		ProblemNodeContainer problemNodeCont(&testBoard);
		problemNodeCont.findAllProblemNodes();

		MoveGenerator moveGen(&testBoard, &problemNodeCont);
		moveGen.setGeneratingName(&name);
		moveGen.setGeneratingPieceBoard(&pieceBoard, false);
	
		BitboardContainer moves = moveGen.getMoves();
		Test::pass( moves == expectedBoard, 
				"incorrect moves outputted for move generation");

		if (!silenced) {
			moves.print();
		}
	}
}

int main() {
	Test::HiveTest::insertPieceTest();
	Test::HiveTest::movePieceTest();
	Test::HiveTest::parseCommandTest();
	Test::BitboardTest::testShiftDirection();
	Test::BitboardTest::testXorWith();
	Test::BitboardTest::testIntersectionWith();
	Test::BitboardTest::testUnionWith();
	Test::BitboardTest::testContainsAny();
	Test::BitboardTest::testFloodFillStep();
	Test::BitboardTest::testFloodFill();
	Test::BitboardTest::testSplit();
	Test::BitboardTest::testSplitIntoConnectedComponents();
	Test::ProblemNodeContainerTest::testFindAllProblemNodes();
	Test::ProblemNodeContainerTest::testRemovePiece(true);
	Test::MoveGeneratorTest::testQueenMoves();
	Test::MoveGeneratorTest::testSpiderMoves();
	Test::MoveGeneratorTest::testGrasshopperMoves();
	Test::MoveGeneratorTest::testBeetleMoves();
	Test::MoveGeneratorTest::testLadybugMoves();
	Test::MoveGeneratorTest::testAntMoves();
}
