#include "Bitboard.h"
#include "MoveGenerator.h"
#include "constants.h"
#include <vector>
#include <unordered_map>
#include <iostream> 

using namespace std;

MoveGenerator::MoveGenerator(Bitboard * allPiecesIn) {
	allPieces = allPiecesIn;
}

// if a given piece complies with the Hive sliding rule, return true
bool checkLegalWalk(Bitboard& allPieces, Bitboard& board, Direction dir) {
	Bitboard CW(board), CCW(board);
	CW.shiftDirection(rotateClockWise(dir));
	CCW.shiftDirection(rotateCounterClockWise(dir));
	return  !(allPieces.containsAny(CW) && allPieces.containsAny(CCW));
}

//  return a bitboard representing all directions a piece can slide to
//  (does not comply with the one hive rule)
Bitboard getLegalWalks(Bitboard& board, Bitboard& allPieces) {
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

Bitboard MoveGenerator::getMoves() {	
	piecesExceptCurrent.initializeTo(*allPieces);
	moves.clear();

	//remove generatingPiece
	piecesExceptCurrent.xorWith(*generatingPieceBoard);
	perimeter = piecesExceptCurrent.getPerimeter();

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
	extraInfo.clear();
	for (Direction dir: hexagonalDirections){
		Bitboard nextPiece(*generatingPieceBoard);

		nextPiece.shiftDirection(dir);
		
		if (extraInfoOn) extraInfo.unionWith(nextPiece);
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
			extraInfo.unionWith(nextPiece);
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
void MoveGenerator::generateLadybugMoves(int depth){
	Bitboard frontier, path, visited, result;
	frontier.initializeTo(*generatingPieceBoard);
	//store extra info for move
	if (extraInfoOn) {
		extraInfo.clear();
		graph.clear();
		graph.root.initializeTo(frontier);
	}
	ladybugStep(frontier, result, path, depth);
	result.intersectionWith(perimeter);
	moves = result;
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

		if (extraInfoOn) 
			graph.assignNode(newFrontier, step);

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

void MoveGenerator::findAntMoves(Bitboard& frontier, Bitboard& visited, Bitboard& ant){
	moves.clear();
	piecesExceptCurrent.initializeTo(*allPieces);

	perimeter.initializeTo(*allPieces);
	perimeter.notIntersectionWith(ant);
	perimeter = perimeter.getPerimeter();

	generateLegalAntMoves(frontier, visited);
	moves.notIntersectionWith(ant);
	moves.intersectionWith(perimeter);
	visited = moves;
}

//TODO: this is so slowwww
void MoveGenerator::generateLegalAntMoves(Bitboard startNodes,  Bitboard visitedNodes) {
	Bitboard frontiers;
	Bitboard visited;
	Bitboard newFrontiers;

	if (startNodes.count()){
		visited = visitedNodes;
		newFrontiers = startNodes;
		for (Bitboard& node: startNodes.splitIntoBitboards()){
			Bitboard test = getLegalWalkPerimeter(node);
			node.notIntersectionWith(*allPieces);
			if( visitedNodes.containsAny(test)) 
				visited.unionWith(node);
		}
	} else {
		newFrontiers = *generatingPieceBoard;
	}
	//perform a flood fill step until it cannot anymore
	//TODO: avoid resplitting the board by using a getWalkPerimeter function that can
	//handle multiple bits at once
	while(newFrontiers.count()) {
		frontiers.initializeTo(newFrontiers);
		newFrontiers.clear();
		for (Bitboard& frontier : frontiers.splitIntoBitboards()) {
			//perform a flood fill step with only legal directions 
			frontier = getLegalWalkPerimeter(frontier);
			frontier.intersectionWith(perimeter);
			frontier.notIntersectionWith(visited);

			// store new visited nodes
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

Bitboard MoveGenerator::getLegalConnectedComponents(Bitboard test) {
	Bitboard components(test);
	Bitboard frontiers;
	if (test.count()) {
		frontiers = test.getRandom();
	}
	Bitboard newFrontiers = frontiers;
	while (newFrontiers.count() ){
		frontiers.clear();
		for (auto frontier : newFrontiers.splitIntoBitboards()) {
			frontier = getLegalWalkPerimeter(frontier);	
			frontier.intersectionWith(test);
			test.notIntersectionWith(frontier);
			frontiers.unionWith(frontier);
		}
		newFrontiers = frontiers;
	}
	components.notIntersectionWith(test);
	return components;
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
Bitboard MoveGenerator::getLegalClimbs( Bitboard& board){
	Bitboard testNeighbors;
	for (auto dir : hexagonalDirections) {
		Bitboard neighbor = getLegalClimb(board, dir);
		testNeighbors.unionWith(neighbor);
	}
	return testNeighbors;
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
			//store extra information for post-processing
			if (extraInfoOn) extraInfo.unionWith(gate);				
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
		//middle bits
		testPerimeter.setBoard(LSB.first, GATES[__builtin_ctzll(LSB.second)]
											   [testPerimeter[LSB.first]]);
		return testPerimeter;
	} else if (LSB.second & 0x1010101010100u) {
		// rightmost bits
		correction = Direction::W;
		LSB.second <<= 1;
	} else if (LSB.second & 0x7e00000000000000u) {
		// upper bits
		correction = Direction::N;
		LSB.second >>= 16;
	} else if (LSB.second & 0x7e) {
		// lower bits
		correction = Direction::S;
		LSB.second <<= 16;
	} else if (LSB.second & 0x80808080808000u) {
		// leftmost bits
		correction = Direction::E;
		LSB.second >>= 1;
	} else {
		//use the slow method only for diagonal corner bits
		Bitboard legalWalks = getLegalWalks(board, *allPieces);
		legalWalks.notIntersectionWith(testPerimeter);
		return legalWalks;
	}

	//since only one board is stored, shift the perimeter to occupy one board
	//get the stored value, then correct
	testPerimeter.shiftDirection(oppositeDirection[correction]);
	if (correction == N || correction == S)
		testPerimeter.shiftDirection(oppositeDirection[correction]);
	testPerimeter.setBoard(LSB.first, GATES[__builtin_ctzll(LSB.second)][testPerimeter[LSB.first]]);
	testPerimeter.shiftDirection(correction);
	if (correction == N || correction == S)
		testPerimeter.shiftDirection(correction);

	return testPerimeter;
}


