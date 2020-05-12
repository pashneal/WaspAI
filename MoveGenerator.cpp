#include "Bitboard.h"
#include "ProblemNode.h"
#include "MoveGenerator.h"
#include "constants.h"
#include <vector>
#include <unordered_map>
#include <iostream> 

using namespace std;

MoveGenerator::MoveGenerator(Bitboard * allPiecesIn , ProblemNodeContainer * problemNodesIn) {
	allPieces = allPiecesIn;
	problemNodes = problemNodesIn;
}

Bitboard MoveGenerator::getMoves() {	
	piecesExceptCurrent.initializeTo(*allPieces);

	moves.clear();

	//remove generatingPiece
	piecesExceptCurrent.xorWith(*generatingPieceBoard);


	perimeter = piecesExceptCurrent.getPerimeter();

	//TODO: remove 
	generateMoves();
	return moves;
}

//I know I should have made separate classes or sum'n ={
void MoveGenerator::generateMoves() {
	switch(*generatingPieceName){
		case GRASSHOPPER:
			generateGrasshopperMoves();
			break;
		case QUEEN:   
			generateQueenMoves();
			break;
		case LADYBUG:
			generateLadybugMoves();
			break;
		case PILLBUG:
			generatePillbugMoves();
			break;
		case MOSQUITO:
			generateMosquitoMoves();
			break;
		case BEETLE:
			generateBeetleMoves();
			break;
		case ANT:
			if (approximate)
				generateApproxAntMoves();
			else 
				generateLegalAntMoves();
			break;
		case SPIDER:
			generateSpiderMoves();
			break;
		default:
		{
			cout << "error 11";
			cout << " if you have reached here may god help you";
			throw 'a';
		}
	}
	
	//the initial location does not count as a move
	//so delete
	moves.notIntersectionWith(*generatingPieceBoard);
}


//TODO:optimize it is so slow
void MoveGenerator::generateGrasshopperMoves(){
	intermediate.clear();
	for (Direction dir: hexagonalDirections){
		Bitboard nextPiece(*generatingPieceBoard);

		nextPiece.shiftDirection(dir);
		
		intermediate.unionWith(nextPiece);
		//check if there is a piece to jump over
		nextPiece.intersectionWith(*allPieces);

		if (nextPiece.count() == 0) {
			//cannot move in this Direction
			//if no piece to jump over
			continue;
		}

		//jump over pieces until it is not possible
		do {
			nextPiece.shiftDirection(dir);
			intermediate.unionWith(nextPiece);
		} while(allPieces -> containsAny(nextPiece));
				
		moves.unionWith(nextPiece);
	}
}

void MoveGenerator::generateQueenMoves(){ 
	Bitboard frontier;
	frontier.initializeTo(*generatingPieceBoard);
	
	Bitboard neighbors = frontier.getPerimeter();
	neighbors.intersectionWith(*allPieces);
	neighbors = neighbors.getPerimeter();

	frontier = getLegalWalkPerimeter(frontier);

	//only keep nodes that are along the board perimeter
	frontier.intersectionWith(perimeter);
	
	//maintain contact with any of original neighbors
	frontier.intersectionWith(neighbors);

	moves.unionWith(frontier);
}


//TODO:optimize currently most expensive function
void MoveGenerator::generateLadybugMoves(){
	Bitboard frontier, path, visited, result;

	frontier.initializeTo(*generatingPieceBoard);

	ladybugStep(frontier, result, path, 0);

	result.intersectionWith(perimeter);

	moves.unionWith(result);
}

void MoveGenerator::ladybugStep(Bitboard& frontier,	
								Bitboard& result, 
								Bitboard& path,
								int step) {
	if (step == 3){
		result.unionWith(frontier);
		return;
	}

	Bitboard newFrontier;
	for (auto dir: hexagonalDirections ){
		newFrontier = getLegalClimb(frontier, dir);
		newFrontier.notIntersectionWith(path);

		if (step < 2) 
			newFrontier.intersectionWith(piecesExceptCurrent);
		if (newFrontier.count() == 0) 
			continue;

		path.unionWith(newFrontier);
		ladybugStep(newFrontier, result, path, step + 1);
		path.notIntersectionWith(newFrontier);
	}

}
void MoveGenerator::generatePillbugMoves(){
	//the Pillbug moves just as a Queen does
	generateQueenMoves();
} 

//any piece in the returned Bitboard can be moved to any empty square in returned bit board
//assumes that they are not pinned
Bitboard MoveGenerator::getPillbugSwapSpaces() {

	Bitboard legalClimbs;
	for (Direction dir : hexagonalDirections) {
		Bitboard legalClimb = getLegalClimb(*generatingPieceBoard, dir);
		legalClimb.notIntersectionWith(*upperLevelPieces);
		legalClimbs.unionWith(legalClimb);
	}
	return legalClimbs;

}
void MoveGenerator::generateMosquitoMoves(){
	//no such thing as MosquitoMoves as it takes the moves from its neighbors
	//and is a bettle on top of the hive (nothing more)
	//takes no moves from adjacent mosquitoes
}

void MoveGenerator::generateBeetleMoves(){
	//TODO: Beetle move generation is lazy


	Bitboard frontier, neighbors;
	frontier.initializeTo(*generatingPieceBoard);
	neighbors = frontier.getPerimeter();
	neighbors.intersectionWith(piecesExceptCurrent);

	//if the piece is adjacent, assume the beetle can climb it
	moves.unionWith(neighbors);

	//set neighbors to perimeter of neighbors
	neighbors = neighbors.getPerimeter();

	frontier = getLegalWalkPerimeter(frontier);

	//maintain contact with at least one of the original neighbors
	frontier.intersectionWith(neighbors);

	Bitboard legalClimbs(frontier);
	legalClimbs.intersectionWith(*upperLevelPieces);

	if (legalClimbs.count() > 1) {
		legalClimbs.clear();
		for (auto dir: hexagonalDirections) {
			Bitboard legalClimb = getLegalClimb(frontier, dir);
			legalClimbs.unionWith(legalClimb);
		}
		frontier.intersectionWith(legalClimbs);
	}


	moves.unionWith(frontier);
}  

//TODO: this is so slowwww
void MoveGenerator::generateLegalAntMoves() {
	//perform a flood fill step until it cannot anymore

	Bitboard frontiers;
	Bitboard visited;
	Bitboard newFrontiers(*generatingPieceBoard);
	while(newFrontiers.count()) {
		frontiers.initializeTo(newFrontiers);
		newFrontiers.clear();
		for (Bitboard& frontier : frontiers.splitIntoBitboards()) {
			//perform a flood fill step with regard to problematic nodes
			frontier = getLegalWalkPerimeter(frontier);
			frontier.intersectionWith(perimeter);
			frontier.notIntersectionWith(visited);

			// store previously visited nodes
			visited.unionWith(frontier);
			// store next locations to search
			newFrontiers.unionWith(frontier);
		}
	}

	moves.unionWith(visited);
}

void MoveGenerator::generateApproxAntMoves() {
	
	moves.unionWith(perimeter);
	moves.notIntersectionWith(*generatingPieceBoard);
}

/*
TODO: make this work when you have time for optimization
void MoveGenerator::generateAntMoves(){
	Bitboard inaccessibleNodes = getInaccessibleNodes(gatesSplit);
	
	moves.unionWith(perimeter);
	moves.notIntersectionWith(inaccessibleNodes);
	moves.notIntersectionWith(*generatingPieceBoard);

}     
*/
void MoveGenerator::generateSpiderMoves(){
	Bitboard frontier, visited, neighbors;

	list <pair<Bitboard, Bitboard>> frontierVisited;
	
	frontier.initializeTo(*generatingPieceBoard);
	frontierVisited.push_back({frontier, visited});

	//TODO: optimize by storing the intermediate search in a 9 KB lookup table

	for (int depth = 0; depth < NUM_SPIDER_MOVES ; depth++) {

		list <pair<Bitboard, Bitboard>> next;

		
		for (auto pairOfBoards : frontierVisited) {

			frontier.initializeTo(pairOfBoards.first);
			visited.initializeTo(pairOfBoards.second);

			//get pieces that touch the frontier
			neighbors = frontier.getPerimeter();
			neighbors.intersectionWith(piecesExceptCurrent);


			visited.unionWith(frontier);
			frontier = getLegalWalkPerimeter(frontier);
			frontier.intersectionWith(perimeter);
			frontier.notIntersectionWith(visited);
			// get the perimeter of original neighbors
			neighbors = neighbors.getPerimeter();

			// make sure to maintain contact with any of the original neighbors
			frontier.intersectionWith(neighbors);

			if (frontier.count()) {

				for (auto& node: frontier.splitIntoBitboards()) {

					//store the node and the path to the node
					pair <Bitboard, Bitboard> pairOfBoards =
					{node, visited};

					next.push_back(pairOfBoards);

				}

			}

		}

		//get next set of frontiers and visited paths
		frontierVisited = next;
	}

	for (auto pairOfBoards : frontierVisited)
		moves.unionWith(pairOfBoards.first);
}  

//TODO: Optimize so you don't have to iterate through every gate separately 
Bitboard MoveGenerator::getInaccessibleNodes(vector <Bitboard> * gates) {
	Bitboard inaccessible;

	for (auto gate: *gates) {
		vector <Bitboard> frontiers;
		vector <Bitboard> initialFrontiers;

		for (auto frontierMap : gate.split()) {
			for (auto board: frontierMap.second) {
				frontiers.push_back(Bitboard({{frontierMap.first, board}}));
			}
		}

		for (auto frontier: frontiers) {
			initialFrontiers.push_back(frontier);
		}

		bool searchResolved[frontiers.size()];
		unsigned long searchResolvedCount = 0;
		int index;

		for (unsigned long i = 0; i < frontiers.size() ; i++) searchResolved[i] = 0;

		while (searchResolvedCount < frontiers.size() - 1) {

			Bitboard visited;
			for (auto frontier = frontiers.begin(); frontier != frontiers.end(); frontier++) {

				if (frontier -> count() == 0) {
					index = frontier - frontiers.begin();
					if (searchResolved[index])
						continue;
					searchResolved[index] = true;
					searchResolvedCount++;
				}
				visited.initializeTo(*frontier);

				//conduct a search and store prev nodes in visited	
				perimeter.floodFillStep(*frontier, visited);

				//TODO: replace with andNot()
				//delete gate nodes from the frontier
				frontier -> unionWith(gate);
				frontier -> xorWith(gate);

				//remove empty board
				frontier -> pruneCache();
			}
		}		

		for (unsigned long i = 0; i < frontiers.size(); i++) {
		
			frontiers[i].intersectionWith(*generatingPieceBoard);

			if (!frontiers[i].equals(*generatingPieceBoard)){
				//TODO: change clear to be a simple xor call

				//if the search concluded before reaching target node
				//the node is in accessible
				inaccessible.unionWith(initialFrontiers[i]);
			}
		}
	}

	return inaccessible;
}

void MoveGenerator::setGeneratingName(PieceName * pieceNameIn) {
	generatingPieceName = pieceNameIn;
}

void MoveGenerator::setGeneratingPieceBoard(Bitboard * b) {
	generatingPieceBoard = b;
}

void MoveGenerator::setUpperLevelPieces(Bitboard * in) {
	upperLevelPieces = in;
}
void MoveGenerator::setPieceStacks(unordered_map <int , deque <pair < PieceColor , PieceName >>> * in) {
	pieceStacks = in;
}

//Optimize TODO
Bitboard MoveGenerator::getInaccessibleNodes(Bitboard gates) {
	vector <Bitboard> gatesVector = {gates};
	return getInaccessibleNodes(&gatesVector);
}


Bitboard MoveGenerator::getLegalClimb( Bitboard& board, Direction dir) {
	Bitboard test(board), gate, CWgate(board), CCWgate(board);

	Direction CW = rotateClockWise(dir);
	CWgate.shiftDirection(CW);
	gate.unionWith(CWgate);

	Direction CCW = rotateCounterClockWise(dir);
	CCWgate.shiftDirection(CCW);
	gate.unionWith(CCWgate);
	
	gate.intersectionWith(*upperLevelPieces);

	test.shiftDirection(dir);
	
	if (gate.count() == 2) {
		int maxPieceHeight = 1;
		if (upperLevelPieces -> containsAny(test)) {
			maxPieceHeight =  pieceStacks->at(test.hash()).size(); 
		}
		if (upperLevelPieces -> containsAny(board) ) {
			int max = pieceStacks ->at(board.hash()).size();
			maxPieceHeight = (max > maxPieceHeight) ? max : maxPieceHeight;
		}

		int minGateHeight = 100;
		int gateCWHeight = pieceStacks ->at(CWgate.hash() ).size();
		int gateCCWHeight = pieceStacks ->at(CCWgate.hash() ).size();

		minGateHeight = (gateCWHeight < gateCCWHeight) ? gateCWHeight : gateCCWHeight;
		if (minGateHeight <= maxPieceHeight) {
			return test;
		}

	}
	return test;
}
Bitboard MoveGenerator::getLegalWalkPerimeter(Bitboard board) {
	if (board.count() != 1) {
		cout << "CANNOT GET LEGAL WALK PERIMETER OF THIS BOARD" << endl;
		cout << board.count() << endl;
		throw 2;
	}
	pair<int, unsigned long long> LSB = board.getLeastSignificantBit();
	Direction correction;
	Bitboard testPerimeter(board);
	testPerimeter = testPerimeter.getPerimeter();
	testPerimeter.intersectionWith(piecesExceptCurrent);
	if (LSB.second & 0x7e7e7e7e7e7e00u) {
		testPerimeter.setBoard(LSB.first, GATES[__builtin_ctzll(LSB.second)]
											   [testPerimeter[LSB.first]]);
		return testPerimeter;
	} else if (LSB.second & 0x1010101010100u) {
		correction = Direction::W;
		LSB.second <<= 1;
	} else if (LSB.second & 0x7e00000000000000) {
		correction = Direction::N;
		LSB.second >>= 16;
	} else if (LSB.second & 0x7e) {
		correction = Direction::S;
		LSB.second <<= 16;
	} else if (LSB.second & 0x80808080808000u) {
		correction = Direction::E;
		LSB.second >>= 1;
	} else {
		int xShift, yShift;
		xShift = __builtin_ctzll(LSB.second) % 8;
		yShift = __builtin_ctzll(LSB.second) / 8;
		xShift = 1-xShift;
		yShift = 1-yShift;

		testPerimeter.shiftDirection(Direction::N , yShift);
		testPerimeter.shiftDirection(Direction::E, xShift);
		testPerimeter.convertToHexRepresentation(Direction::NE, yShift);
		
		testPerimeter.setBoard(LSB.first, GATES[__builtin_ctzll(512u)][testPerimeter[LSB.first]]);

		testPerimeter.shiftDirection(Direction::S , yShift);
		testPerimeter.shiftDirection(Direction::W, xShift);
		testPerimeter.convertToHexRepresentation(Direction::SW, yShift);
		return testPerimeter;
	}

	testPerimeter.shiftDirection(oppositeDirection[correction]);
	if (correction == N || correction == S)
		testPerimeter.shiftDirection(oppositeDirection[correction]);
	testPerimeter.setBoard(LSB.first, GATES[__builtin_ctzll(LSB.second)][testPerimeter[LSB.first]]);
	testPerimeter.shiftDirection(correction);
	if (correction == N || correction == S)
		testPerimeter.shiftDirection(correction);

	return testPerimeter;
}
