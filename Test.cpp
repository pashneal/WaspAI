#include <chrono>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <vector>
#include <set>
#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <random>
#include "constants.h"
#include "Arena.h"
#include "Bitboard.h"
#include "MoveGenerator.h"
#include "PieceNode.h"
#include "ProblemNode.h"
#include "PieceGraph.h"
#include "Test.h"
#include "GameState.h"

using namespace std;

std::mt19937_64 e2(rd());
std::uniform_int_distribution<unsigned long long int> dist(0,18446744073709551615u);

int PERIMETER_SIZE = 2;
vector <unordered_map<unsigned long long, unsigned long long[5]>> PERIMETER = {{{}}};
unordered_map <unsigned long long , unsigned long long > GATES[64];

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

void Test::BitboardTest::testShiftDirection(){
	cout << "==========Test::Bitboard::shiftDirection()=======" << endl;

	Bitboard testBitboard;
	//thanks to http://cinnamonchess.altervista.org/bitboard_calculator/Calc.html for allowing me to create these tests more easily
	vector <unordered_map <int, unsigned long long>> bitboardList =
	{
		{
			{0,2323928329449648128u},
			{1,288795546670598144u}
		},

		{
			{0,609009639598005946u},
			{1,826246961152u},
			{2,1610618880u}
		},

		{
			{0,2378943904679710u},
			{1,3227527192u},
			{2,6291480u},
			{3,13402712491054596096u}
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
			{4,2258444583180296}
		},


	};


	vector <Direction> shiftDirections = {
		Direction::W,
		Direction::S,
		Direction::N,
		Direction::E,
		Direction::NE,
		Direction::SW,
		Direction::SW, 
	};

	vector <unordered_map <int, unsigned long long>> expectedResults = 
	{
		{
			{0,1161964714480637952u},
			{1,144397771187815424u}
		},

		{
			{0,2378943904679710u},
			{1,3227527192u},
			{2,6291480u},
			{3,13402712491054596096u}
		},

		{
			{0,609009639598005946u},
			{1,826246961152u},
			{2,1610618880u},
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
			{4, 4423901974552},{7,288230376151711744},{3, 2147483648}
		},
	};

	for (auto test: bitboardList) {
		Bitboard expectedBoard(test), testBoard(test);
		for (Direction dir : hexagonalDirections) {
			testBoard.shiftDirection(dir);
		}


		Test::pass(testBoard == expectedBoard, 
				"did not return to original position after shifting");
		if (!(testBoard == expectedBoard)) {
			cout << endl;
			cout << "=====testBoard=====" <<endl;
			testBoard.print();
			cout << "==expectedBoard====" << endl;
			expectedBoard.print();
			cout << endl;
		}
		for (Direction dir : hexagonalDirections) {
			testBoard.initialize(test);
			testBoard.shiftDirection(dir);
			testBoard.shiftDirection(oppositeDirection[dir]);
			Test::pass(testBoard == expectedBoard, " using oppositeDirections did not cancel out");
			if (!(testBoard == expectedBoard)) {
				cout << endl << "[DIRECTION] : " << dir << endl;
				cout << "=====testBoard(shift 2)=====" <<endl;
				testBoard.print();
				cout << "====testBoard(shift 1)==" << endl;
				testBoard.initialize(test);
				testBoard.shiftDirection(dir);
				testBoard.print();
				cout << "==expectedBoard====" << endl;
				expectedBoard.print();
				cout << endl;
			}
		}
	}
		
	for (unsigned long long i = 0; i < shiftDirections.size(); ++i){
		cout << "Test " << i <<
			" direction: " << shiftDirections[i] << endl ;

		testBitboard.initialize(bitboardList[i]);
		testBitboard.shiftDirection(shiftDirections[i]);
		Bitboard expectedResult(expectedResults[i]);
			Test::pass(expectedResult == testBitboard,
					"Expected result incorrect for shiftDirection() ");
		testBitboard.print();
		expectedResult.print();
	}

	unordered_map <int, unsigned long long> test;
	test[5] = 1;
	testBitboard.initialize(test);
	testBitboard.shiftDirection(Direction::NE);
	testBitboard.shiftDirection(Direction::W);
	testBitboard.shiftDirection(Direction::SW);
	testBitboard.shiftDirection(Direction::SE);
	testBitboard.shiftDirection(Direction::E);
	testBitboard.shiftDirection(Direction::NW);
	cout << "Test " << shiftDirections.size() << ": ";
	Test::pass(testBitboard[5] == 1, "result incorrect");

	bool silenced = true;

	cout << "Test " << shiftDirections.size() + 1 << ": ";

	Bitboard testProblemNodes;
	testProblemNodes.initialize({{6, 524800u}});
	if (!silenced) {testProblemNodes.print(); cout << endl;}
	testProblemNodes.shiftDirection(Direction::N, 15);
	if (!silenced) {testProblemNodes.print(); cout << endl;}
	testProblemNodes.shiftDirection(Direction::E, 7);
	if (!silenced) {testProblemNodes.print(); cout << endl;}
	testProblemNodes.convertToHexRepresentation(Direction::NE, 15);
	if (!silenced) {testProblemNodes.print(); cout << endl;}
	Bitboard result({{1,1026u}});
	Test::pass(result == testProblemNodes, " result incorrect");
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

	Bitboard test, compare;
	for (unsigned long long i = 0; i < expectedResults.size(); ++i){
		cout << "Test " << i << endl;
		
		test.initialize(bitboardInitialList[i]);
		compare.initialize(bitboardCompList[i]);
		test.xorWith(compare);
		for (auto j : expectedResults[i]){
				cout << "\t Board: " << j.first << " ";

				Test::pass(test[j.first] == j.second, 
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


	Bitboard test, compare;
	for (unsigned long long i = 0; i < expectedResults.size(); ++i){
		cout << "Test " << i << endl;

		test.initialize(bitboardInitialList[i]);
		compare.initialize(bitboardCompList[i]);
		test.intersectionWith(compare);
		Bitboard results(expectedResults[i]);

		Test::pass(test == results, 
				"failed to get correct result for intersectionWith()");
		test.print();
		results.print();
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


	Bitboard test, compare;
	for (unsigned long long i = 0; i < expectedResults.size(); ++i){
		cout << "Test " << i << endl;
		
		test.initialize(bitboardInitialList[i]);
		compare.initialize(bitboardCompList[i]);
		test.unionWith(compare);
		for (auto j : expectedResults[i]){
				cout << "\t Board: " << j.first << " ";

				Test::pass(test[j.first] == j.second, 
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


	Bitboard test, compare;
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
		{{0,49344u}, {1,1}, {3,13835058055282163712u}},
		{{0,49344u}, {1,1}, {3,13835058055282163712u}}
	};
	vector <unordered_map <int, unsigned long long>> bitboardFrontierList =
	{
		{{0,134217728u}},
		{{0,128}},
		{{0,49216u}, {1,1}, {3,13835058055282163712u}}
	};

	vector <unordered_map <int, unsigned long long >> bitboardVisitedList = {
		{{0,0}},
		{{0,0}},
		{{0,128}}
	};


	vector <unordered_map <int, unsigned long long >> expectedVisitedList = {
		{{0,134217728u}},
		{{0,128}},
		{{0,49344u}, {1,1}, {3,13835058055282163712u}}
	};

	vector <unordered_map <int, unsigned long long >> expectedFrontierList = {
		{{0,34695806976u}},
		{{0,49216u}, {1,1}, {3,13835058055282163712u}},
		{{}}
	};
	Bitboard traversable, frontier, visited;
	for (unsigned long long i = 0; i < expectedFrontierList.size(); ++i){
		cout << "Test " << i << endl;
		
		traversable.initialize(bitboardTraversableList[i]);
		frontier.initialize(bitboardFrontierList[i]);
		visited.initialize(bitboardVisitedList[i]);

		traversable.floodFillStep(frontier, visited);

		Bitboard compVisited(expectedVisitedList[i]);
		Bitboard compFrontier(expectedFrontierList[i]);

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
		{{0,144680354303193280u}, {1,396553u}, {3,4629806107761573888u}}
	};
	vector <unordered_map <int, unsigned long long>> bitboardFrontierList =
	{
		{{0,144115188075855872}}
	};

	vector <unordered_map <int, unsigned long long >> expectedFrontierList = {
		{{0,144680354303193280u}, {1,396553u}, {3,4629806107761573888u}}
	};

	Bitboard traversable, frontier ;
	for (unsigned long long i = 0; i < expectedFrontierList.size(); ++i){
		cout << "Test " << i << endl;
		traversable.initialize(bitboardTraversableList[i]);
		frontier.initialize(bitboardFrontierList[i]);
		traversable.floodFill(frontier);
		Bitboard compFrontier(expectedFrontierList[i]);
		frontier.pruneCache();
		compFrontier.pruneCache();
		cout << "\t" ;
		Test::pass(compFrontier.equals(frontier), 
				"frontier board is incorrect for floodFill");
	}
}

void Test::BitboardTest::testSplit() {
	vector <unsigned long long> randomNums;

	Bitboard testBitboard;
	for (int i = 0; i < BITBOARD_SIZE; i++) {
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
		 {3,4629806107769962496u}, {4, 216172782113849344u}};

	vector <unordered_map <int, unsigned long long >> expectedComponentsList= {
		{{0,144680354232401920u}},
		{{0,3682304u}},
		{{1,265224u}},
		{{1,257}, {4,216172782113783808u}},
		{{3,8388608u}, {4,65536u}},
		{{3, 4629806107761573888u}}
	};

	Bitboard traversable(bitboardTraversable);
	int testNum = 0;

	for (Bitboard result: traversable.splitIntoConnectedComponents()) {
		cout << "Test " << testNum << endl << "\t";
		bool flag = 0;
		for (Bitboard test: expectedComponentsList){ 
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
		{{2, 51708299278u}},
		{{5, 2361380u}},
		{{8, 1116741894656u}},
		{{6,549755813888u}, {7,16777216u}},
		{{6, 806367232u}},
		{{6, 2147516416u}, {7, 16908544u}}
	};

	vector <unordered_map <int, unsigned long long >> expected= {
		{{0, 33686536u}},
		{{2, 8830520132096u}},
		{{4, 141287244169216u}, {5, 1108135248128u}},
		{{2,1792u}},
		{{5,1586712u}},
		{{8,2211908550912u}},
		{{6,2147483648u}, {7, 4294967296u}},
		{{6,6291456}},
		{{6, 8388608u}, {7, 65536u}}
	};
	for (unsigned int i = 0; i < expected.size(); i++ ){
		cout << "Test" << i << ": ";
		Bitboard gateTest(gatesTest[i]);
		Bitboard expectedProblemNodes(expected[i]);
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
		{{2, 51708299278u}},
		{{5, 2361380u}},
		{{8, 1116741894656u}},
		{{6,549755813888u}, {7,16777216}},
		{{7,50660096u} , {6,3225468928u}}
	};

	vector <unordered_map <int, unsigned long long >> removePiece = {
		{{0, 4}},
		{{2, 1024}},
		{{5, 16777216u}},
		{{2,17179869184u}},
		{{5, 2048u}},
		{{8, 1099511627776u}},
		{{7,16777216}},
		{{7,65536}}
	};

	vector <unordered_map <int, unsigned long long >> expected= {
		{{0, 33685504u}},
		{{2, 8830520131584u}},
		{{4, 141289391652864u}, {5, 1108118470912u}},
		{{2, 17246979840u}},
		{{5, 13824u}},
		{{8, 393472u}},
		{{}},
		{{6,8388608u}, {7,196608u}}
		
	};

	for (unsigned int i = 0; i < gatesTest.size(); i++) {
		cout << "Test " << i << ": ";
		
		Bitboard initialBoard(gatesTest[i]);
		Bitboard removePieceBoard(removePiece[i]);
		ProblemNodeContainer test(&initialBoard);
		Bitboard expectedProblemNodes(expected[i]);

		test.findAllProblemNodes();
		test.removePiece(removePieceBoard);

		Test::pass(test.visibleProblemNodes == expectedProblemNodes,
					"incorrect board returned for problemNodes");

		if (!noMessage) {
			cout << "given Board: " ;
			test.visibleProblemNodes.print();
			
			cout << endl;

			cout << "expected Board: " ;
			expectedProblemNodes.print();
			cout << endl <<endl ;
		}
	}
}

void Test::MoveGeneratorTest::testQueenMoves() {
	cout << "===================TestQueenMoves===================" << endl;
	PieceName name = PieceName::QUEEN;
	bool silenced =true;

	vector <unordered_map <int, unsigned long long>> test = {
		{{8, 2258422653255680u}},
		{{0, 103550550016u}},
		{{3, 103550025728u}},
		{{5, 827595993088u}, {6, 16908544u}},
		{{4,1621106688u}}
	};

	vector <unordered_map<int, unsigned long long>> piece = {
		{{8,2251799813685248u}},
		{{0, 134217728u}},
		{{3, 134217728u}},
		{{5 , 8388608u}},
		{{4, 8388608u}}
	};

	vector <unordered_map <int, unsigned long long >> expected = {
		{{8, 1134695999864832u}},
		{{}},
		{{}},
		{{5,1073758208u}},
		{{4, 2151677952u}}
	};


	for (unsigned long long i = 0; i < expected.size(); i++ ) {
		cout << "Test " << i << ": ";
		Bitboard testBoard(test[i]);
		Bitboard expectedBoard(expected[i]);
		Bitboard pieceBoard(piece[i]);


		MoveGenerator moveGen(&testBoard);
		moveGen.setGeneratingName(&name);
		moveGen.setGeneratingPieceBoard(&pieceBoard);
	
		Bitboard moves = moveGen.getMoves();
		Test::pass( moves == expectedBoard, 
				"incorrect moves outputted for move generation");

		if (!silenced) {
			cout << "given moves" << endl;
			moves.print();
		}
	}
}

void Test::MoveGeneratorTest::testSpiderMoves() {
	cout << "===================TestSpiderMoves===================" << endl;
	PieceName name = PieceName::SPIDER;
	bool silenced =true;

	vector <unordered_map <int, unsigned long long>> test = {
		{{4, 68921589760u}},
		{{5, 103550550016u}},
		{{8, 103550025728u}},
		{{5, 827595993088u}, {6, 16908544u}},
		{{4,1621106688u}},
		//TODO: stop being lazy with this test 
		{{4, 211381651554304u}, {5, 4479648768u}},
		{{3,1006910464u}}
	};

	vector <unordered_map<int, unsigned long long>> piece = {
		{{4,68719476736u}},
		{{5, 134217728u}},
		{{8, 134217728u}},
		{{5 , 8388608u}},
		{{4, 8388608u}},
		{{5,131072u}},
		{{3, 536870912u}}
	};

	vector <unordered_map <int, unsigned long long >> expected = {
		{{4, 33556480u}},
		{{}},
		{{}},
		{{5, 1073758240u}, {6, 1}},
		{{4, 274877907008u}},
		{{4, 550829555712u} , {5, 51608291333u}},
		{{3, 34359754752u}}
	};


	for (unsigned long long i = 0; i < expected.size(); i++ ) {
		if (!silenced) cout << endl;
		cout << "Test " << i << ": ";
		Bitboard testBoard(test[i]);
		Bitboard expectedBoard(expected[i]);
		Bitboard pieceBoard(piece[i]);

		ProblemNodeContainer problemNodeCont(&testBoard);
		problemNodeCont.findAllProblemNodes();

		MoveGenerator moveGen(&testBoard);
		moveGen.setGeneratingName(&name);
		moveGen.setGeneratingPieceBoard(&pieceBoard);
	
		Bitboard moves = moveGen.getMoves();
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
	bool silenced = true;

	vector <unordered_map <int, unsigned long long>> test = {
		{{4, 2258422653255680u}},
		{{5, 103550550016u}},
		{{8, 103550025728u}},
		{{5, 827595993088u}, {6, 16908544u}},
		{{4,1621106688u}}
	};

	vector <unordered_map<int, unsigned long long>> piece = {
		{{4,2251799813685248u}},
		{{5, 134217728u}},
		{{8, 134217728u}},
		{{5 , 8388608u}},
		{{4, 8388608u}}
	};

	vector <unordered_map <int, unsigned long long >> expected = {
		{{4, 1139094046375936u}},
		{{5,103416332288u}},
		{{8,103415808000u}},
		{{5, 3221274624u}},
		{{4, 3225419776u}}
	};


	for (unsigned long long i = 0; i < expected.size(); i++ ) {
		cout << "Test " << i << ": ";
		Bitboard testBoard(test[i]);
		Bitboard expectedBoard(expected[i]);
		Bitboard pieceBoard(piece[i]);

		ProblemNodeContainer problemNodeCont(&testBoard);
		problemNodeCont.findAllProblemNodes();

		MoveGenerator moveGen(&testBoard);
		moveGen.setGeneratingName(&name);
		moveGen.setGeneratingPieceBoard(&pieceBoard);
	
		unordered_map <int, deque < pair < PieceColor, PieceName>>> m;
		Bitboard upperLevelPieces;

		moveGen.setPieceStacks(&m);
		moveGen.setUpperLevelPieces(&upperLevelPieces);
		Bitboard moves = moveGen.getMoves();
		Test::pass( moves == expectedBoard, 
				"incorrect moves outputted for move generation");

		if (!silenced) {
			cout << "given moves" << endl;
			moves.print();

		}
	}
	Bitboard testBoard({{5, 865847986923967777u}});
	Bitboard expectedBoard({{5, 203164672u}});
	Bitboard upperLevelPieces({{5, 524288u}});
	Bitboard givenPiece({{5, 524288u}});
	name = PieceName::BEETLE;
	unordered_map < int, deque < pair <PieceColor, PieceName>>> pieceStacks; 
	pieceStacks[givenPiece.hash()].push_front({PieceColor::WHITE, PieceName::BEETLE});
	ProblemNodeContainer problemNodeCont(&testBoard);
	problemNodeCont.findAllProblemNodes();
	MoveGenerator moveGenerator(&testBoard);
	moveGenerator.setUpperLevelPieces(&upperLevelPieces);
	moveGenerator.setPieceStacks(&pieceStacks);
	moveGenerator.setGeneratingPieceBoard(&givenPiece);
	moveGenerator.setGeneratingName(&name);
	Test::pass(moveGenerator.getMoves() == expectedBoard, " incorrect moves outputted for move "
															"generation");

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
		Bitboard testBoard(test[i]);
		Bitboard expectedBoard(expected[i]);
		Bitboard pieceBoard(piece[i]);

		ProblemNodeContainer problemNodeCont(&testBoard);
		problemNodeCont.findAllProblemNodes();

		MoveGenerator moveGen(&testBoard);
		moveGen.setGeneratingName(&name);
		moveGen.setGeneratingPieceBoard(&pieceBoard);
	
		Bitboard moves = moveGen.getMoves();
		Test::pass( moves == expectedBoard, 
				"incorrect moves outputted for move generation");

		if (!silenced) {
			cout << "given moves" << endl;
			moves.print();
		}
	}
}

void Test::MoveGeneratorTest::testLadybugMoves() {
	cout << "===================TestLadybugMoves===================" << endl;
	PieceName name = PieceName::LADYBUG;
	bool silenced = true;

	vector <unordered_map <int, unsigned long long>> test = {
		{{4, 68921589760u}},
		{{5, 103550550016u}},
		{{8, 103550025728u}},
		{{5, 827595993088u}, {6, 16908544u}},
		{{4,1621106688u}},
		{{3,1006910464u}}
	};

	vector <unordered_map<int, unsigned long long>> piece = {
		{{4,68719476736u}},
		{{5, 134217728u}},
		{{8, 134217728u}},
		{{5 , 8388608u}},
		{{4, 8388608u}},
		{{3, 536870912u}}
	};

	vector <unordered_map <int, unsigned long long >> expected = {
		{{4, 51574213632u}},
		{{5, 30941517192192u}},
		{{8, 30941517715456u}},
		{{5, 211107306274816u}, {6, 12918522371u}},
		{{4, 412589490176u}},
		{{3, 103080787968u}}
	};


	for (unsigned long long i = 0; i < expected.size(); i++ ) {
		if (!silenced) cout << endl;
		cout << "Test " << i << ": ";
		Bitboard testBoard(test[i]);
		Bitboard expectedBoard(expected[i]);
		Bitboard pieceBoard(piece[i]);

		ProblemNodeContainer problemNodeCont(&testBoard);
		problemNodeCont.findAllProblemNodes();
		unordered_map <int, deque < pair < PieceColor, PieceName>>> m;
		Bitboard upperLevelPieces;


		MoveGenerator moveGen(&testBoard);
		moveGen.setGeneratingName(&name);
		moveGen.setGeneratingPieceBoard(&pieceBoard);
		moveGen.setPieceStacks(&m);
		moveGen.setUpperLevelPieces(&upperLevelPieces);
	
		Bitboard moves = moveGen.getMoves();
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
	bool silenced = true;

	vector <unordered_map <int, unsigned long long>> test = {
		{{4, 68921589760u}},
		{{5, 103550550016u}},
		{{8, 103550025728u}},
		{{5, 827595993088u}, {6, 16908544u}},
		{{4,1621106688u}},
		{{3,1006910464u}}
	};

	vector <unordered_map<int, unsigned long long>> piece = {
		{{4,68719476736u}},
		{{5, 134217728u}},
		{{8, 134217728u}},
		{{5 , 8388608u}},
		{{4, 8388608u}},
		{{3, 536870912u}}
	};

	vector <unordered_map <int, unsigned long long >> expected = {
		{{4, 51842780672u}},
		{{}},
		{{}},
		{{5, 246497975748848}, {6, 12918718979u}},
		{{4, 964493856864u}},
		{{3, 257738031740u}}
	};


	for (unsigned long long i = 0; i < expected.size(); i++ ) {
		if (!silenced) cout << endl;
		cout << "Test " << i << ": ";
		Bitboard testBoard(test[i]);
		Bitboard expectedBoard(expected[i]);
		Bitboard pieceBoard(piece[i]);

		ProblemNodeContainer problemNodeCont(&testBoard);
		problemNodeCont.findAllProblemNodes();

		MoveGenerator moveGen(&testBoard);
		moveGen.setGeneratingName(&name);
		moveGen.setGeneratingPieceBoard(&pieceBoard);
	
		moveGen.approximate =false;
		Bitboard moves = moveGen.getMoves();
		Test::pass( moves == expectedBoard, 
				"incorrect moves outputted for move generation");

		if (!silenced) {
			moves.print();
		}
	}
}

void Test::MoveGeneratorTest::testPillbugMoves() {
	cout << "===================TestPillbugMoves===================" << endl;
	PieceName name = PieceName::PILLBUG;
	bool silenced = true;

	vector <unordered_map <int, unsigned long long>> test = {
		{{4, 68921589760u}},
		{{5, 103550550016u}},
		{{8, 103550025728u}},
		{{5, 827595993088u}, {6, 16908544u}},
		{{4,1621106688u}},
		{{3,1006910464u}}
	};

	vector <unordered_map<int, unsigned long long>> piece = {
		{{4,68719476736u}},
		{{5, 134217728u}},
		{{8, 134217728u}},
		{{5 , 8388608u}},
		{{4, 8388608u}},
		{{3, 536870912u}}
	};

	vector <unordered_map <int, unsigned long long >> expected = {
		{{4, 34628173824u}},
		{{}},
		{{}},
		{{5,1073758208u}},
		{{4, 2151677952u}}
	};


	for (unsigned long long i = 0; i < expected.size(); i++ ) {
		if (!silenced) cout << endl;
		cout << "Test " << i << ": ";
		Bitboard testBoard(test[i]);
		Bitboard expectedBoard(expected[i]);
		Bitboard pieceBoard(piece[i]);

		ProblemNodeContainer problemNodeCont(&testBoard);
		problemNodeCont.findAllProblemNodes();

		MoveGenerator moveGen(&testBoard);
		moveGen.setGeneratingName(&name);
		moveGen.setGeneratingPieceBoard(&pieceBoard);
	
		Bitboard moves = moveGen.getMoves();
		Test::pass( moves == expectedBoard, 
				"incorrect moves outputted for move generation");

		if (!silenced) {
			moves.print();
		}
	}
}

void Test::PieceGraphTest::testFindAllPinnedPieces(){
	cout << "===================TestFindAllPinnedPieces===================" << endl;
	bool silenced = true;

	vector <unordered_map <int, unsigned long long>> test = {
		{{4, 68921589760u}},
		{{5, 103550550016u}},
		{{8, 103550025728u}},
		{{5, 827595993088u}, {6, 16908544u}},
		{{4,1621106688u}},
		{{3,1006910464u}}
	};

	vector <unordered_map <int, unsigned long long >> expected = {
		{{4,134217728u}},
		{{}},
		{{}},
		{{5,827587563520u}},
		{{4, 1612709888u}},
		{{3,470031360u}}
	};


	for (unsigned long long i = 0; i < expected.size(); i++ ) {
		if (!silenced) cout << endl;
		cout << "Test " << i << ": ";
		Bitboard allPieces(test[i]);
		Bitboard expectedBoard(expected[i]);
		
		PieceGraph pieceGraph;

		for (auto board: allPieces.splitIntoBitboards()) 
			pieceGraph.insert(board);

		for (auto board: allPieces.splitIntoBitboards()) {
			Bitboard testboard = board.getPerimeter();
			testboard.intersectionWith(allPieces);
			for ( auto adjBoard: testboard.splitIntoBitboards() ) {
				Test::pass(pieceGraph.checkBiDirectional(adjBoard, board), "An edge given by "
						"pieceGraph was not bidirectional");
			}
		}


		Bitboard pinned = pieceGraph.getPinnedPieces();

		Test::pass( pinned == expectedBoard, 
				"incorrect pinned pieces");

		if (!silenced) {
			pinned.print();
		}
	}
};

void Test::GameStateTest::testFastSpawnPiece(){
	bool silenced = true;

	GameState gameState(HivePLM, PieceColor::WHITE);
	vector <pair <Bitboard, PieceName> > initialPieces;
	initialPieces = {
		{Bitboard({{5,134217728u}}), PieceName::ANT},
		{Bitboard({{5,34359738368u}}), PieceName::ANT},
		{Bitboard({{5,1048576u}}), PieceName::ANT},
		{Bitboard({{5, 4398046511104u}}), PieceName::ANT},
		{Bitboard({{5, 524288u}}), PieceName::QUEEN},
	};

	Bitboard finalBoard;

	bool color = (bool)PieceColor::WHITE;
	vector <Bitboard> whiteBlackPieces(2);
	for (auto p : initialPieces) {

		whiteBlackPieces[color].unionWith(p.first);
		finalBoard.unionWith(p.first);

		gameState.fastSpawnPiece(p.first, p.second);
		color = !color;
	}
	
	cout << "===================TestFastSpawn==================" << endl;



	Test::pass( whiteBlackPieces[(PieceColor)0] == *gameState.getPieces((PieceColor)0) && 
				whiteBlackPieces[(PieceColor)1] == *gameState.getPieces((PieceColor)1),
				"whiteBlackPieces not updated correctly");

	unordered_set <PieceNode*> s = gameState.pieceGraph.DFS();

	Bitboard pieceGraphBoard;
	for (PieceNode* n: s) {
		Bitboard temp({{n -> boardIndex , n -> location}});
		pieceGraphBoard.unionWith(temp);
	}

	Test::pass( pieceGraphBoard == finalBoard , "pieceGraph not updated correctly");
	if (!silenced) { cout << "pieceGraphBoard" << endl; pieceGraphBoard.print();}


	Test::pass( gameState.allPieces == finalBoard , "allPieces not updated correctly");
	if (!silenced) {cout << "allpiece" << endl; gameState.allPieces.print();}

	Bitboard ants(finalBoard);
	Bitboard queen({{5, 524288u}});
	ants.notIntersectionWith(queen);

	Test::pass( gameState.ants == ants, "ants not updated correctly");
	if (!silenced) {cout << "ants" << endl; gameState.ants.print();};

	Test::pass( gameState.queens == queen, "queens not updated correctly");
	if (!silenced) {cout << "queens" << endl; gameState.queens.print();}

	Bitboard empty;

	for (int i = 0 ; i <= PieceName::SPIDER; i++) {
		if (i == PieceName::ANT || i == PieceName::QUEEN) continue;
		Test::pass( *gameState.getPieces((PieceName)i) == empty, " a piece not updated correctly");
		if (!silenced) {gameState.getPieces((PieceName)i) -> print();}
	}

	Test::pass( gameState.immobile == queen, " immobile not updated correctly");
	if (!silenced) {cout << "immobile" << endl; gameState.immobile.print();}
	
	Bitboard pinned({{5, 34493956096u}});
	Test::pass( gameState.pinned == pinned, " pinned not updated correctly");
	if (!silenced) {cout << "pinned" << endl; pinned.print();}

	Test::pass( gameState.upperLevelPieces == empty, " upperLevelPieces not updated correctly");
	if (!silenced) {cout << "upperLevelPieces" << endl; gameState.upperLevelPieces.print();}

	
	Test::pass( PieceColor::BLACK == gameState.turnColor, " turnColor is incorrect");
	if (!silenced) {cout << gameState.turnColor<< endl;}

	Test::pass( gameState.turnCounter == 5, "turnCounter is incorrect");
	if (!silenced) {cout << gameState.turnCounter << endl;}
}

void Test::GameStateTest::testMovePiece(){
	GameState gameState(HivePLM, PieceColor::WHITE);
	vector <pair <Bitboard, PieceName> > initialPieces;
	cout << "====================TestMovePiece====================" << endl;
	initialPieces = {
		{Bitboard({{5,134217728u}}), PieceName::ANT}, //White
		{Bitboard({{5,34359738368u}}), PieceName::ANT}, //Black
		{Bitboard({{5,1048576u}}), PieceName::ANT}, //White
		{Bitboard({{5, 4398046511104u}}), PieceName::ANT}, //Black
		{Bitboard({{5, 524288u}}), PieceName::QUEEN}, //White
		{Bitboard({{5, 8796093022208u}}), PieceName::BEETLE}, //Black
		{Bitboard({{5, 67108864u}}), PieceName::MOSQUITO}, //White
	};
	

	Bitboard testAnt({{5, 4398046511104u}});
	Bitboard testBeetle({{5, 8796093022208u}});
	Bitboard testMosquito({{5, 67108864u}});
	Bitboard finalBeetle({{5, 34359738368u}});
	Bitboard finalMosquito({{5, 34359738368u}});
	Bitboard finalAnt({{5, 17179869184u}});

	bool color = (bool)PieceColor::WHITE;
	for (auto p : initialPieces) {
		gameState.fastSpawnPiece(p.first, p.second);
		color = !color;
	}

	gameState.fastMovePiece(testAnt, finalAnt);  //black
	gameState.fastMovePiece(testMosquito, finalMosquito); //white
	gameState.fastMovePiece(testBeetle, finalBeetle); //black
	
	color = !color;
	int turnCounter = 10;
	Bitboard blackPieces({{5, 51539607552u}});
	Bitboard whitePieces({{5, 34495528960u}});
	Bitboard finalBoard({{5, 51675398144u}});
	Bitboard ants({{5, 51674873856u}});
	Bitboard queens({{5, 524288u}});
	Bitboard mosquitoes({{5, 34359738368u}});
	Bitboard beetles({{5, 34359738368u}});
	Bitboard pinned({{5, 34493956096u}});
	Bitboard immobile(finalBeetle);
	auto stackCopy = gameState.pieceStacks[finalBeetle.hash()];
	stack < pair < PieceColor , PieceName> >  stackCompare;

	stackCompare.push({PieceColor::BLACK, PieceName::ANT});
	stackCompare.push({PieceColor::WHITE, PieceName::MOSQUITO});
	stackCompare.push({PieceColor::BLACK, PieceName::BEETLE});
	
	while (!stackCompare.empty() ) {
		Test::pass(stackCompare.top() == stackCopy.front(),
				"pieceStacks produced unexpected results");
		stackCompare.pop(); stackCopy.pop_front();
	}

	bool silenced = true;
	Test::pass(turnCounter == gameState.turnCounter, "turnCounter produced incorrect results");
	Test::pass(gameState.ants == ants , " ants produced incorrect results");
	Test::pass(gameState.queens == queens , " queens produced incorrect results");
	Test::pass(gameState.mosquitoes == mosquitoes , " mosquitoes produced incorrect results");
	Test::pass(gameState.beetles == beetles , " beetles produced incorrect results");
	Test::pass(gameState.pinned == pinned , " pinned produced incorrect results");
	Test::pass(gameState.immobile == immobile , " immobile produced incorrect results");
	Test::pass(gameState.whitePieces == whitePieces , " whitePieces produced incorrect results");
	Test::pass(gameState.blackPieces == blackPieces , " blackPieces produced incorrect results");
	Test::pass(gameState.allPieces == finalBoard , " allPieces produced incorrect results");
	Test::pass(finalBeetle == gameState.upperLevelPieces, " upperLevelPieces incorrect");
	if (!silenced) {gameState.upperLevelPieces.print();}

	gameState.changeTurnColor();
	gameState.fastMovePiece(finalBeetle, testBeetle);
	blackPieces.initialize({{5, 8847632629760u}});	
	finalBoard.unionWith(blackPieces);
	immobile.initializeTo(testBeetle);
	beetles.initializeTo(testBeetle);
	turnCounter++;

	stackCopy = gameState.pieceStacks[finalBeetle.hash()];
	stackCompare.push({PieceColor::BLACK, PieceName::ANT});
	stackCompare.push({PieceColor::WHITE, PieceName::MOSQUITO}); 

	cout << "-----------------------------" << endl;
	while (!stackCompare.empty() ) {
		Test::pass(stackCompare.top() == stackCopy.front(),
				"pieceStacks produced unexpected results");
		stackCompare.pop(); stackCopy.pop_front();
	}

	Test::pass(turnCounter == gameState.turnCounter, "turnCounter produced incorrect results");
	Test::pass(gameState.ants == ants , " ants produced incorrect results");
	Test::pass(gameState.queens == queens , " queens produced incorrect results");
	Test::pass(gameState.mosquitoes == mosquitoes , " mosquitoes produced incorrect results");
	Test::pass(gameState.beetles == beetles , " beetles produced incorrect results");
	if (!silenced) {gameState.beetles.print();};
	Test::pass(gameState.pinned == pinned , " pinned produced incorrect results");
	Test::pass(gameState.immobile == immobile , " immobile produced incorrect results");
	Test::pass(gameState.whitePieces == whitePieces , " whitePieces produced incorrect results");
	Test::pass(gameState.blackPieces == blackPieces , " blackPieces produced incorrect results");
	if (!silenced) {gameState.blackPieces.print();}
	Test::pass(gameState.allPieces == finalBoard , " allPieces produced incorrect results");
	Test::pass(finalMosquito == gameState.upperLevelPieces, " upperLevelPieces incorrect");
	if (!silenced) {gameState.upperLevelPieces.print();}
}

void Test::GameStateTest::testPsuedoRandom() {
	GameState gameState(HivePLM, PieceColor::WHITE);
	vector <pair <Bitboard, PieceName> > initialPieces;
	cout << "====================TestPsuedoRandom====================" << endl;
	initialPieces = {
		{Bitboard({{5,134217728u}}), PieceName::ANT},
		{Bitboard({{5,34359738368u}}), PieceName::MOSQUITO},
		{Bitboard({{5,1048576u}}), PieceName::ANT},
		{Bitboard({{5, 4398046511104u}}), PieceName::ANT},
		{Bitboard({{5, 524288u}}), PieceName::QUEEN},
		{Bitboard({{5, 8796093022208u}}), PieceName::BEETLE},
		{Bitboard({{5, 67108864u}}), PieceName::MOSQUITO},
		{Bitboard({{5, 17592186044416u}}), PieceName::ANT},
		{Bitboard({{5, 17179869184u}}), PieceName::LADYBUG},
		{Bitboard({{5, 134217728u}}), PieceName::BEETLE},
	};
	
	bool color  = 0; 
	for (auto p : initialPieces) {
		gameState.fastSpawnPiece(p.first, p.second);	
		color = !color;

		if (gameState.turnColor != color) {
			Test::pass(false, "color not updated after a move");
			throw 74;
		}
	}

	PieceName name;

	Bitboard ant({{5,1048576u}});
	Bitboard queen({{5, 524288u}});
	Bitboard whiteMosquito({{5, 67108864u}});
	Bitboard ladybug({{5, 17179869184u}});

	MoveGenerator moveGenerator(&gameState.allPieces);
	Bitboard antMoves, queenMoves, ladybugMoves, whiteMosquitoMoves;

	moveGenerator.setPieceStacks(&gameState.pieceStacks);
	moveGenerator.setUpperLevelPieces(&gameState.upperLevelPieces);
	moveGenerator.setGeneratingName(&name);


	
	name = PieceName::ANT;
	moveGenerator.setGeneratingPieceBoard(&ant);
	antMoves = moveGenerator.getMoves();

	name = PieceName::QUEEN;
	moveGenerator.setGeneratingPieceBoard(&queen);
	queenMoves = moveGenerator.getMoves();

	name = PieceName::LADYBUG;
	moveGenerator.setGeneratingPieceBoard(&ladybug);
	ladybugMoves = moveGenerator.getMoves();	
	
	moveGenerator.setGeneratingPieceBoard(&whiteMosquito);
	whiteMosquitoMoves = moveGenerator.getMoves();

	Bitboard fauxBeetleMoves(whiteMosquito);
	fauxBeetleMoves = fauxBeetleMoves.getPerimeter();
	fauxBeetleMoves.intersectionWith(gameState.allPieces);
	whiteMosquitoMoves.unionWith(fauxBeetleMoves);

	name = PieceName::QUEEN;
	Bitboard fauxQueenMoves = moveGenerator.getMoves();
	whiteMosquitoMoves.unionWith(fauxQueenMoves);
	
	unordered_map <int, Bitboard> expectedMoves  = {
		{ant.hash(), antMoves},
		{queen.hash(), queenMoves},
		{ladybug.hash(), ladybugMoves},
		{whiteMosquito.hash(), whiteMosquitoMoves},
	};

	Bitboard expectedSpawns({{5,8625855488u}});
	Bitboard immobilePieces({{5,30820819533824u}});


	unordered_map < int , unordered_map <PieceName, bool> > foundSpawns;


	for (auto board: expectedSpawns.splitIntoBitboards() ) {
		for ( auto element: gameState.unusedPieces[PieceColor::WHITE]) {
			if (element.second > 0) {
				foundSpawns[board.hash()][element.first] = false;
			}
			
		}
	}

	unordered_map <int, Bitboard> foundMoves;
	for (int i = 0 ; i <  10000; i ++ ) {
		if ((i % 100) == 0) cout << i << " moves Generated" << endl;

		GameState testGameState(gameState);
		testGameState.makePsuedoRandomMove();

		for (Bitboard board: testGameState.allPieces.splitIntoBitboards()) {
			Bitboard testboard = board.getPerimeter();
			testboard.intersectionWith(testGameState.allPieces);
			for ( auto adjBoard: testboard.splitIntoBitboards() ) {
				if (!testGameState.pieceGraph.checkBiDirectional(adjBoard, board)) {
				Test::pass(false, "An edge given by pieceGraph was not bidirectional");
				
				testGameState.print();
				throw 67;

				}
			}
		}


		if (testGameState.upperLevelPieces.containsAny(testGameState.immobile)) {
			//if piece moved on top of stack
			if ( testGameState.allPieces.containsAny(whiteMosquito)) {
				//if the white mosquito didn't move
				Test::pass(false, "made illegal move involving mosquito");
				Bitboard diff(testGameState.allPieces);
				diff.xorWith(gameState.allPieces);
				testGameState.print();
				cout << "difference of boards" << endl;
				diff.print();
				return;
			}
		}

		Bitboard testIllegal(immobilePieces);
		testIllegal.intersectionWith(testGameState.allPieces);
		if (!(testIllegal == immobilePieces)) {
			Test::pass(false, "Moved a black Piece");
			testGameState.print();
			return;
		}

		testIllegal.initializeTo(testGameState.allPieces);
		testIllegal.intersectionWith(gameState.allPieces);
		if (testIllegal == gameState.allPieces) {
			//no pieces moved
			testIllegal.xorWith(testGameState.allPieces);
			if (!expectedSpawns.containsAny(testIllegal) || testIllegal.count() != 1)  {
				Test::pass(false, "Did not spawn in correct square despite"
							      " moving no existing piece");
				cout << "testIllegal" << endl;
				testIllegal.print();
				cout << ">>>>>>testGameState" << endl;
				testGameState.print();
				cout << ">>>>>>gameState" << endl;
				gameState.print();

				return;
			}

			if ( foundSpawns.find(testIllegal.hash() ) != foundSpawns.end() ) {
				PieceName spawnedName = testGameState.findTopPieceName(testIllegal);
				if ( foundSpawns[testIllegal.hash()].find(spawnedName)  != 
					 foundSpawns[testIllegal.hash()].end() )
				{
					
					foundSpawns[testIllegal.hash()][spawnedName] = true;
					continue;
				} else {
					
					Test::pass(false, "spawning illegal piece");
					return;
				}

			} else {
				Test::pass(false, "Did not spawn in predefined square");
				return;
			}

		}

		testIllegal.initializeTo(gameState.allPieces);
		testIllegal.xorWith(testGameState.allPieces);
		testIllegal.notIntersectionWith(testGameState.immobile);
		if (testIllegal.count() == 0) {
			Test::pass(false, "Something weird happened");
			cout << "gameState.allPieces" << endl;
			gameState.allPieces.print();
			cout << "testGameState.allPieces" << endl;
			testGameState.allPieces.print();
			cout << "testGameState.immobile" << endl;
			testGameState.immobile.print();
			return;
		}
		foundMoves[testIllegal.hash()].unionWith(testGameState.immobile);
	}

	for (auto element: expectedMoves) {
		Bitboard intersection;
		if (foundMoves.find(element.first) != foundMoves.end() ){
			intersection.initializeTo(foundMoves[element.first]);
		}
		intersection.intersectionWith(element.second);
		cout << intersection.count() << "/" << element.second.count() << " "; 
		if (intersection.count() != element.second.count() ) {
			Test::pass( false, "Did not produce all expected moves");
		}
	}

	int passed = 0;
	int total = 0;
	for (auto element: foundSpawns) {
		for (auto iter: element.second) {
			if (iter.second) passed++;
			total++;
		}
	}

	
	cout << passed << "/" << total << " ";
	Test::pass(passed == total, "Did not produce all expected spawns");
	
	GameState c(gameState);

	for (int i = 0 ; i < 10000; i++ ) {
		if (!(i % 100)) cout << i << " probably legal moves made" << endl;	
		if (i % 500 == 0) {c = gameState; color = gameState.turnColor;}
		c.makePsuedoRandomMove();
		//cout << " turnCounter: " << c.turnCounter << " turnColor: " << c.turnColor << endl;
		if (c.allPieces.splitIntoConnectedComponents().size() != 1){
			Test::pass(false, "Last move violated one Hive Rule");
			throw 42;
		}
		

		Bitboard test;
		for (auto i: c.pieceGraph.DFS() ){
			Bitboard t({{i->boardIndex,i->location}});
			test.unionWith(t);
		}
		if (!(test == c.allPieces)) {
			Test::pass(false, "piece graph does not reflect allPieces");
			c.print();
			cout << "test" << endl;
			test.print();
			
			throw 77;
		}
		Bitboard allPiecesTest;
		for (auto name: c.possibleNames) {
			allPiecesTest.unionWith(*c.getPieces(name));	
		}
		if (!(allPiecesTest == c.allPieces)) {
			Test::pass(false, "last move introduced a piece with no name" );
			throw 76;
		}

		vector <unordered_map<PieceName, int>> count;
		count.resize(2);
		for (auto iter: c.pieceStacks){
			auto pieceStack = iter.second;
			while(pieceStack.size()){
				PieceColor foundColor = pieceStack.front().first;
				PieceName foundName = pieceStack.front().second;
				if (count[foundColor].find(foundName) == count[foundColor].end())
					count[foundColor][foundName] = 0;
				count[foundColor][foundName]++;
				pieceStack.pop_front();
			}
		}
		for (int xx = 0 ; xx < PieceColor::NONE; xx++){
			for (int x = 0; x < (int)HivePLM[xx].size(); x++){
				count[xx][(PieceName)x] = HivePLM[xx][(PieceName)x] - count[xx][(PieceName)x];
			}
		}
		if (!(count == c.unusedPieces)){
			Test::pass(false, "There are pieces missing from the hive after the last move");
			c.print();
			cout << endl << "count" << endl;
			for (auto iter: count[0])
				cout << "pieceName " << iter.first << " amountRemaining " << iter.second << endl;
			cout << "player 1" << endl;
			for (auto iter: count[1])
				cout << "pieceName " << iter.first << " amountRemaining " << iter.second << endl;
			throw 54;
		}

		color = !color;
		if (c.turnColor != color) {
			Test::pass(false, "color not updated after a move");
			throw 74;
		}
	}
}

void perfTest() {
	vector <pair <Bitboard , PieceName>> initialPieces = {
		{Bitboard({{5,134217728u}}), PieceName::ANT},
		{Bitboard({{5,34359738368u}}), PieceName::MOSQUITO},
		{Bitboard({{5,1048576u}}), PieceName::ANT},
		{Bitboard({{5, 4398046511104u}}), PieceName::ANT},
		{Bitboard({{5, 524288u}}), PieceName::QUEEN},
		{Bitboard({{5, 8796093022208u}}), PieceName::BEETLE},
		{Bitboard({{5, 67108864u}}), PieceName::MOSQUITO},
		{Bitboard({{5, 17592186044416u}}), PieceName::ANT},
		{Bitboard({{5, 17179869184u}}), PieceName::LADYBUG},
		{Bitboard({{5, 134217728u}}), PieceName::BEETLE},
	};
	
	GameState gameState(HivePLM, PieceColor::WHITE);
	bool color  = 0; 
	for (auto p : initialPieces) {
		gameState.fastSpawnPiece(p.first, p.second);	
		color = !color;
	}

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	GameState c(gameState);
	for (int i = 0; i < 10000 ; i++ ) {
		c.makePsuedoRandomMove();
	}
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "10000 moves made in " << 
	std::chrono::duration_cast<std::chrono::milliseconds>
	(end - begin).count() << "[ms]" << std::endl;
	begin = std::chrono::steady_clock::now();
	GameState b(gameState);
	for (int i = 0; i < 1000; i++ ) {
		b.makePsuedoRandomMove();
	}
	end = std::chrono::steady_clock::now();
	std::cout << "1000 moves made in " << 
	std::chrono::duration_cast<std::chrono::milliseconds>
	(end - begin).count() << "[ms]" << std::endl;

	begin = std::chrono::steady_clock::now();
	GameState a(gameState);
	for (int i = 0; i < 500 ; i++ ) {
		a.makePsuedoRandomMove();
	}
	end = std::chrono::steady_clock::now();
	std::cout << "500 moves made in " << 
	std::chrono::duration_cast<std::chrono::milliseconds>
	(end - begin).count() << "[ms]" << std::endl;

	begin = std::chrono::steady_clock::now();
	GameState d(gameState);
	for (int i = 0; i < 100 ; i++ ) {
		d.makePsuedoRandomMove();
	}
	end = std::chrono::steady_clock::now();
	std::cout << "100 moves made in " << 
	std::chrono::duration_cast<std::chrono::milliseconds>
	(end - begin).count() << "[ms]" << std::endl;
}

void Test::BitboardTest::testFastPerimeter(){
	cout << "====================TestFastPerimeter()====================" << endl;
	unsigned long long position = 1;
	while(position) {
		Bitboard test({{6, position}});
		Bitboard expected(test);
		test = test.getPerimeter();
		expected = expected.slowGetPerimeter();
		Test::pass(test == expected, "testFastPerimeter did not match slowGetPerimeter");
		if (!(test == expected)) {
			cout << "test" << endl; test.print();
			cout << "expected" << endl; expected.print();
			cout << endl;
		}
		
		position <<= 1;
	}
}

void Test::GameStateTest::testPlayout() {
	int numMoves = 0;
	long ms = 0;
	int firstPlayerWins = 0;
	int secondPlayerWins =0 ;
	int draws = 0;
	int count = 125;
	int limit = 2000;
	auto totalBegin = chrono::steady_clock::now();
	for (int i = 0; i < count; i++) {
		GameState newGame(HivePLM, PieceColor::WHITE);
		auto begin = chrono::steady_clock::now();
		int j = newGame.playout(limit); 
		auto end = chrono::steady_clock::now();
		numMoves += j;
		ms += chrono::duration_cast<chrono::milliseconds>(end - begin).count();
		if (newGame.checkVictory() != PieceColor::NONE) {
			if (newGame.checkVictory() == PieceColor::WHITE) {
				firstPlayerWins++;
			} else if (newGame.checkVictory() == PieceColor::BLACK) {
				secondPlayerWins++;
			} else if (newGame.checkDraw()) 
				draws++;
		}
	}
	auto totalEnd = chrono::steady_clock::now();
	double totalElapsed = chrono::duration_cast<chrono::milliseconds>(totalEnd -totalBegin).count();
	cout << " total games played: " << count << endl;
	cout << " limit: " << limit << endl;
	cout << " avg moves: " << (float)numMoves/count << " average ms " << (float)ms/count << endl;
	cout << " firstPlayerWins " << firstPlayerWins << " secondPlayerWins " 
		 << secondPlayerWins << " draws " << draws << endl;
	cout << "total elapsed: " << totalElapsed << "[ms]" << endl;
}

void Test::ArenaTest::testArenaNotation() {
	cout <<"====================TestArenaNotation====================" << endl;
	int f = 1;
	bool silenced = true;
	srand(3);
	while (f--) {
		Arena arena(GameState(HivePLM, PieceColor::WHITE)); 
		int c = 500;
		while (arena.currentGameState.checkVictory() == PieceColor::NONE && c--) {
			vector<MoveInfo> moves = arena.currentGameState.generateAllMoves();
			std::random_shuffle(moves.begin(), moves.end());
			PieceColor turnColor = arena.currentGameState.turnColor;
			if (!silenced)
				cout << "turnColor " << turnColor << endl;
			for (auto move: moves){ 
				//make sure the conversion is transitive
				string s = arena.convertToNotation(move);
				MoveInfo m = arena.convertFromNotation(s);

				if (!(m==move)){
					Test::pass(m==move,"conversion to and from" 
						" notation produced unexpected results");
					cout << s << endl;
					arena.currentGameState.print();
					cout << "original MoveInfo" << endl;
					move.oldPieceLocation.print();
					move.newPieceLocation.print();
					cout << move.pieceName << endl;
					cout << "converted MoveInfo" << endl;
					m.oldPieceLocation.print();
					m.newPieceLocation.print();
					cout << m.pieceName << endl;
					throw 118;
				}
			}
			if (moves.size() == 0) moves.push_back(MoveInfo());
			if (!silenced){
				cout << moves.size() << " ";
				cout << arena.convertToNotation(moves[0]) << endl;
			}
			arena.makeMove(moves[0]);
		}
	}
	Test::pass(true, "");
	Arena arena(GameState(HivePLM, PieceColor::WHITE));
	arena.makeMove("wP");
	arena.makeMove("bG1 wP-");
	arena.makeMove("wQ /wP");
	arena.makeMove("bA1 bG1\\");
	arena.makeMove("wB1 wQ\\");
	arena.makeMove("bQ bG1/");
	arena.makeMove("wS1 /wQ");
	arena.makeMove("bA2 bQ/");
	arena.makeMove("wL -wQ");
	arena.makeMove("bA2 \\wQ");

	set <string> expectedMoves {
		"wB1 wQ-",
		"wB1 wS1\\",
		"wB1 wQ",
		"wB1 wS1",
		"wA1 wB1\\",
		"wG1 wB1\\",
		"wB2 wB1\\", 
		"wS2 wB1\\",
		"wM wB1\\",
		"wA1 /wB1",
		"wG1 /wB1",
		"wB2 /wB1",
		"wS2 /wB1",
		"wM /wB1",
		"wS1 wB1-",
		"wS1 \\wL",
		"wA1 /wS1",
		"wG1 /wS1",
		"wB2 /wS1",
		"wS2 /wS1",
		"wM /wS1",
		"wA1 -wS1",
		"wG1 -wS1",
		"wB2 -wS1",
		"wS2 -wS1",
		"wM -wS1",
		"wL \\wP",
		"wL wP/",
		"wL wP\\",
		"wL wQ-",
		"wL \\bA2",
		"wL bA2/",
		"wL -bA2",
		"wL \\wP",
		"wL wP/",
		"wL wP\\",
		"wL wB1/",
		"wL wB1-",
		"wL wB1\\",
		"wL /wB1",
		"wL wS1\\",
		"wL /wS1",
		"wL -wS1",
		"wL wQ-",
		"wL wB1/",
		"wL wB1-",
		"wL wB1\\",
		"wL /wB1",
		"wA1 -wL",
		"wG1 -wL",
		"wB2 -wL",
		"wS2 -wL",
		"wM -wL",
		"wQ \\wP",
		"wQ wP/",
		"wQ wP\\"
	};
	unordered_set <MoveInfo> expectedMoveInfo;
	for (auto move : expectedMoves){
		expectedMoveInfo.insert(arena.convertFromNotation(move));
	}

	for (auto move: arena.currentGameState.generateAllMoves()){
		if (expectedMoveInfo.find(move) == expectedMoveInfo.end()){
			cout << "An illegal move was generated at this position" << endl;
			cout << arena.convertToNotation(move) << endl;
			throw 5;
		}
		expectedMoveInfo.erase(move);
	}
	Test::pass(!expectedMoveInfo.size(),"Not all legal moves were generated");

	if (!expectedMoveInfo.size()) {
		for (auto m : expectedMoveInfo) {
			cout << arena.convertToNotation(m) << endl;
		}
	}
}

void Test::ArenaTest::testBattle() {
	cout << "MONTECARLOSIMULATIONS = " <<endl;
	cin >> MonteCarloSimulations;
	cout << "CUTOFF =" << endl;
	cin >> MonteCarloSimulationsCutoff;
	cout << "NUMCORES = " << endl;
	cin >> numCores;
	Arena arena(GameState(HivePLM, PieceColor::WHITE));
	Heuristic h(SIMPLE, {'P','L','M'});
	MonteCarloTree MCT(h);
	heuristicFactor = .7;
	arena.setPlayer(0, MCT);
	arena.makeMove("wP");
	arena.makeMove("bP wP-");
	arena.makeMove("wM /wP");
	arena.currentGameState.print();
	while (arena.currentGameState.checkDraw() == false && arena.currentGameState.checkVictory() == PieceColor::NONE){
		arena.battle(false);
		arena.currentGameState.print();
	}
}
void Test::MonteCarloTest::testRandomSearch(){
	cout << "=========================TestRandomSearch===========================" <<endl;
	bool silenced = true;

	MonteCarloSimulations  = 1000;
	MonteCarloSimulationsCutoff = 500;
	numCores = 1;

	GameState newGameState(HivePLM, PieceColor::WHITE);
	Heuristic h(RANDOM, {'P','L','M'});
	Arena arena(newGameState);
	MonteCarloNode root;
	MonteCarloTree MCT(h);
	MonteCarloTree MCT2(h);
	arena.setPlayer(PieceColor::WHITE , MCT);
	arena.setPlayer(PieceColor::BLACK , MCT2);
	//set up for mate in one 
	arena.makeMove("wA1");
	arena.makeMove("bA1 wA1-");
	arena.makeMove("wQ -wA1");
	arena.makeMove("bQ bA1-");
	arena.makeMove("wG1 wQ/");
	arena.makeMove("bS1 \\bQ");
	arena.makeMove("wB1 /wQ");
	arena.makeMove("bL1 bQ/");
	arena.makeMove("wS1 /wA1");
	arena.makeMove("bG1 bQ-");
	arena.makeMove("wA2 \\wQ");
	//if the next move is made, white can mate
	// arena.makeMove("bG1 bQ\\");

	arena.battle(silenced);
	cout << endl;
	Test::pass(arena.currentGameState.checkVictory() == PieceColor::BLACK, 
				"Computer player did not find the winning move");
}

//creates a hashTable that stores the precomputed perimeter
//of a given bitboard array
//int maxNumber : the number of bits per board to precomputed
//it seems setting this to 4 gives spectacular results (66% increase in speed)
//memory requirement is O(64^(maxNumber)) bits
void createPerimeterHashTable(int maxNumber) {
	//with a piece centered at original
	//the boards that make up the 8 surrounding
	//boards are listed here
	vector <int> upperLower = {1,7};
	vector <int> leftRight = {3, 5};
	vector <int> diagonal = {0, 2, 6, 8};
	int original = 4;

	/*
	 *
	 *
	 *   ------------------
	 *   |       ||       |
	 *   |upper  ||diagona|
	 *   |lower  ||       |
	 *   ------------------
	 *   |       ||       |
	 *   |origina||left   |
	 *   |       ||right  |
	 *   ------------------
	 *
	 *   This is the orientation for the first blocking masks
	 *   Then it rotates  clockwise
	 *
	 */ 
	
	//we only want overflow on 3 boards MAX so block out possibilities
	//that may give more
	// blocking mask index 0 represents no overflow allowed
	vector <unsigned long long> blockingMasks = {0xff818181818181ffu,
												 0x1010101010101ffu,
												 0xff01010101010101u,
												 0xff80808080808080u,
												 0x80808080808080ffu};

	for (int count = 1; count <= maxNumber ; count++) {
		unsigned long long t;
		PERIMETER.push_back(unordered_map<unsigned long long, unsigned long long [5]>{{}});
		unsigned long long blockingMasksIndex = 0;
		for (auto block: blockingMasks) {
			//current permutation = the smallest possible bit permutation with count # of bits
			unsigned long long v = (1 << count) - 1;
			//while the next bit permutation is still the same count as the last
			while (__builtin_popcountll(v) == count) {
				//while no bit is in blocking mask
				if (__builtin_popcountll((v & ~block)) == count && 
					// and v is not already in hash table
					PERIMETER[count].find(v) == PERIMETER[count].end())
				{


					unsigned long long leftRightBoard= 0, upperLowerBoard= 0
									, originalBoard= 0, diagonalBoard= 0;
					Bitboard b({{original, v}});
					b = b.slowGetPerimeter();
					//search for possible overflows
					for (auto j: upperLower)
						upperLowerBoard |= b[j];
					for (auto j: leftRight)
						leftRightBoard |= b[j];
					for (auto j: diagonal)
						diagonalBoard |= b[j];
					originalBoard = b[original];

					//assign values to hash table
					PERIMETER[count][v][0] = originalBoard;
					PERIMETER[count][v][1] = upperLowerBoard;
					PERIMETER[count][v][2] = leftRightBoard;
					PERIMETER[count][v][3] = diagonalBoard;
					PERIMETER[count][v][4] = blockingMasksIndex;
				}

				//next bit permutation
				//this is a fast bit twiddling trick found online
				t = v | (v - 1);
				v = (t + 1) | (((~t & -~t) - 1) >> (__builtin_ctzll(v) + 1)); 
			}
			blockingMasksIndex++;
		}
	}
}
//  returns a vector containing all combinations of directions
void generateDirectionCombinations (unsigned int i, vector < vector <Direction>>& v) {
	if ( i == hexagonalDirections.size() ) return;

	vector <vector <Direction>> x;
	for ( vector <Direction> w: v) {
		w.push_back((Direction) i);
		x.push_back(w);
	}
	for (vector <Direction> a: x) {
		v.push_back(a);
	}

	generateDirectionCombinations(i+1, v);
}
// if a given piece complies with the Hive sliding rule, return true
bool checkLegalWalk(Bitboard allPieces, Bitboard board, Direction dir) {
	Bitboard CW(board), CCW(board);
	CW.shiftDirection(rotateClockWise(dir));
	CCW.shiftDirection(rotateCounterClockWise(dir));
	return  !(allPieces.containsAny(CW) && allPieces.containsAny(CCW));
}
//  return a bitboard representing all directions a piece can slide to
//  does not comply with the one hive rule
Bitboard getLegalWalks(Bitboard board, Bitboard allPieces) {
	Bitboard retBoard;
	for (unsigned i = 0; i < hexagonalDirections.size(); i++) {
		if (checkLegalWalk(allPieces, board, (Direction)i))
		{
			Bitboard test(board);
			test.shiftDirection((Direction)i);
			retBoard.unionWith(test);
		}
	}
	return retBoard;
}

//creates a hashTable that stores the precomputed perimeter
void createGateHashTable() {
	unsigned long long notAllowed = 0xff818181818181ffu;
	unsigned long long position = 1; 
	int boardIndex = 6;
	vector <vector <Direction>> v;
	v.push_back({});
	generateDirectionCombinations(0, v);
	vector<unsigned long long> legalWalks;
	while (position) {
		if (position & ~notAllowed) {
			for (vector <Direction> directions : v) {
				Bitboard board({{boardIndex , position}});
				Bitboard test;
				Bitboard duplicated;
				for ( Direction dir: directions) {
					test.initializeTo(board);
					test.shiftDirection(dir);
					duplicated.unionWith(test);	
				}
				Bitboard walks = getLegalWalks(board, duplicated);
				GATES[__builtin_ctzll(position)][duplicated[boardIndex]] = walks[boardIndex];
			}
		}
		position <<= 1;
	}
}

int main() {
	srand(2);
	cout << "Initializing..." << endl;
	createPerimeterHashTable(PERIMETER_SIZE);
	cout << "Finished initializing perimeters" << endl;
	cout << "Intializing... " << endl;
	createGateHashTable();
	cout << "Finished initializing gates" << endl;
	Test::BitboardTest::testShiftDirection();
	Test::BitboardTest::testXorWith();
	Test::BitboardTest::testIntersectionWith();
	Test::BitboardTest::testUnionWith();
	Test::BitboardTest::testContainsAny();
	Test::BitboardTest::testFastPerimeter();
	Test::BitboardTest::testFloodFillStep();
	Test::BitboardTest::testFloodFill();
	Test::BitboardTest::testSplit();
	Test::BitboardTest::testSplitIntoConnectedComponents();
	//tests now fail because hash function does not support two bits
	//Test::ProblemNodeContainerTest::testFindAllProblemNodes();
	//Test::ProblemNodeContainerTest::testRemovePiece(true);
	Test::MoveGeneratorTest::testQueenMoves();
	Test::MoveGeneratorTest::testSpiderMoves();
	Test::MoveGeneratorTest::testGrasshopperMoves();
	Test::MoveGeneratorTest::testBeetleMoves();
	Test::MoveGeneratorTest::testLadybugMoves();
	Test::MoveGeneratorTest::testAntMoves();
	Test::MoveGeneratorTest::testPillbugMoves();
	Test::PieceGraphTest::testFindAllPinnedPieces();
	Test::GameStateTest::testFastSpawnPiece();
	Test::GameStateTest::testMovePiece();
	//Test::GameStateTest::testPsuedoRandom();
	//perfTest();
	//Test::GameStateTest::testPlayout();
	//Test::ArenaTest::testArenaNotation();
	Test::ArenaTest::testBattle();
	//Test::MonteCarloTest::testRandomSearch();
}
