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

		class PieceTest {
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
				static void testBitboardBoundings();
				static void testShiftDirection();
				static void testXorWith();
				static void testIntersectionWith();
				static void testUnionWith();
				static void testContainsAny();
		};

		static void pass(bool, string);
};


