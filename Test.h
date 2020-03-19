
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
				static void testShiftDirection();//done
				static void testWarpPiece();
				static void testRecenterBoard();
				static void testFloodFillAlongPath();//untested
				static void testFindConnectedCompBFS();
				static void testFindPinned();
				static void testFindRings();
				static void testFindGates();
				static void testFindDoors();
				static void testInsertPiece();
				static void testRemovePiece();
				static void testFindPinnedDependencies();
				static void testBitboardBoundings();
				// be sure to pin pieces with beetles on them
				static void testDraw();
				static void testWin();
				static void testOneHiveRuleWithFloodFill();	
		};

		static void pass(bool, string);
};


