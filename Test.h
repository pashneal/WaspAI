#pragma once
#include <iostream> 

using namespace std;

class Test {
	public:
		static bool silent;
		class HiveTest {
			public:
				static void insertPieceTest();
				static void movePieceTest();
				static void parseCommandTest();
		};

		class MoveGeneratorTest {
			public:
				static void testAntMoves();
				static void testGrasshopperMoves();
				//pay extra attention to beetle tests that "float" the beetle
				static void testBeetleMoves();
				static void testQueenMoves();
				static void testPillbugMoves();
				static void testMosquitoMoves();
				static void testLadybugMoves();
				static void testSpiderMoves();
		};


		class BitboardTest {
			public:
				static void testMovePiece();
				static void testShiftDirection();
				static void testXorWith();
				static void testIntersectionWith();
				static void testUnionWith();
				static void testContainsAny();
				static void testFloodFillStep();
				static void testFloodFill();
				static void testSplit();
				static void testSplitIntoConnectedComponents();
		};

		class ProblemNodeContainerTest {
			public:
				static void testFindAllProblemNodes();
				static void testRemovePiece(bool);
		};

		class PieceGraphTest {
			public: 
				static void testFindAllPinnedPieces();
		};

		class GameStateTest {
			public:
				static void testMovePiece();
				static void testInsertPiece();
				static void testFastSpawnPiece();
				static void testPsuedoRandom();
		};

		static void pass(bool, string);
};


