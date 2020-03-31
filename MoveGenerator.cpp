#include "Bitboard.h"
#include "ProblemNode.h"
#include "MoveGenerator.h"
#include "constants.h"
#include <vector>
#include <unordered_map>
#include <iostream> 

using namespace std;

MoveGenerator::MoveGenerator(BitboardContainer * allPiecesIn , ProblemNodeContainer * problemNodesIn) {
	allPieces = allPiecesIn;
	problemNodes = problemNodesIn;
}

BitboardContainer MoveGenerator::getMoves() {	

	piecesExceptCurrent.initializeTo(*allPieces);

	moves.clear();

	//remove generatingPiece
	piecesExceptCurrent.xorWith(*generatingPieceBoard);

	perimeter = piecesExceptCurrent.getPerimeter();
	
	problemNodes -> removePiece(*generatingPieceBoard);
	generateMoves();
	problemNodes -> insertPiece(*generatingPieceBoard);
	
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
			generateAntMoves();
			break;
		case SPIDER:
			generateSpiderMoves();
			break;
		default:
			cout << "error 11";
			cout << " if you have reached here may god help you";
			throw 11;
			break;
	}
	
	//the initial location does not count as a move
	//so delete
	moves.unionWith(*generatingPieceBoard);
	moves.xorWith(*generatingPieceBoard);
}


//TODO:optimize it is so slow
void MoveGenerator::generateGrasshopperMoves(){

	
	for (Direction dir: hexagonalDirections){

		BitboardContainer nextPiece(*generatingPieceBoard);

		nextPiece.shiftDirection(dir);
		
		//check if there is a piece to jump over
		nextPiece.intersectionWith(*allPieces);
		if (nextPiece.count() == 0) {
			//cannot move in this Direction
			//if no piece to jump over
			continue;
		}

		bool pieceExists = true;
		nextPiece.initializeTo(*generatingPieceBoard);
		nextPiece.shiftDirection(dir);

		while (pieceExists) {

			//see if there is a piece to jump over
			nextPiece.shiftDirection(dir);
			pieceExists = allPieces -> containsAny(nextPiece);

		}

		moves.unionWith(nextPiece);
	}

}

void MoveGenerator::generateQueenMoves(){ 
	BitboardContainer frontier;
	frontier.initializeTo(*generatingPieceBoard);
	
	BitboardContainer neighbors = frontier.getPerimeter();
	neighbors.intersectionWith(*allPieces);
	neighbors = neighbors.getPerimeter();

	//if the piece is on a problematic node
	if (problemNodes -> contains(*generatingPieceBoard)){
		//get allowed directions to travel
		frontier = problemNodes -> getPerimeter(frontier);

	} else { 
		//search all directions around frontier
		frontier = frontier.getPerimeter();
	}

	//only keep nodes that are along the board perimeter
	frontier.intersectionWith(perimeter);
	
	//maintain contact with any of original neighbors
	frontier.intersectionWith(neighbors);

	moves.unionWith(frontier);
}

void MoveGenerator::generateLadybugMoves(){
	
	BitboardContainer frontier, visited;
	frontier.initializeTo(*generatingPieceBoard);

	//must first make two moves on top of other pieces 
	allPieces -> floodFillStep(frontier, visited);
	allPieces -> floodFillStep(frontier, visited);

	//must make a move that is not on top other pieces
	frontier = frontier.getPerimeter();
	frontier.notIntersectionWith(*allPieces);

	moves.unionWith(frontier);

} 

void MoveGenerator::generatePillbugMoves(){
	//the Pillbug moves just as a Queen does
	generateQueenMoves();
} 

void MoveGenerator::generateMosquitoMoves(){
	//no such thing as MosquitoMoves as it takes the moves from its neighbors
	//and is a bettle on top of the hive (nothing more)
	//takes no moves from adjacent mosquitoes
}

void MoveGenerator::generateBeetleMoves(){
	//TODO: Beetle move generation is lazy
	//Further analysis is necessary in order to see
	//If moves atop the hive are legal

	BitboardContainer traversable;


	//since the beetle can move atop the other pieces
	//include those pieces as traversable

	BitboardContainer frontier;
	frontier.initializeTo(*generatingPieceBoard);

	traversable.initializeTo(frontier);

	//set traversable to neighbors
	traversable = traversable.getPerimeter();
	traversable.intersectionWith(*allPieces);

	//union with every node adjacent to neighbors
	traversable.duplicateBoard(hexagonalDirections);

	//if the piece is on a problematic node
	if (problemNodes -> contains(*generatingPieceBoard)){
		
		//get allowed directions to travel
		frontier = problemNodes -> getPerimeter(frontier);

	} else { 

		//search all directions around frontier
		frontier = frontier.getPerimeter();
	}
	//only keep directions that are traversable
	frontier.intersectionWith(traversable);

	moves.unionWith(frontier);

}  

void MoveGenerator::generateAntMoves(){
	BitboardContainer inaccessibleNodes = getInaccessibleNodes(gatesSplit);

	//since inaccessible nodes are a subset of perimeter
	//xor them out after adding perimeter
	moves.unionWith(perimeter);
	moves.xorWith(inaccessibleNodes);

	//remove initial location
	moves.unionWith(*generatingPieceBoard);
	moves.xorWith(*generatingPieceBoard);
}     

void MoveGenerator::generateSpiderMoves(){

	BitboardContainer frontier, visited, neighbors;

	list <pair<BitboardContainer, BitboardContainer>> frontierVisited;
	
	frontier.initializeTo(*generatingPieceBoard);
	frontierVisited.push_back({frontier, visited});

	//TODO: optimize by storing the intermediate search in a 9 KB lookup table

	for (int depth = 0; depth < NUM_SPIDER_MOVES ; depth++) {

		list <pair<BitboardContainer, BitboardContainer>> next;

		
		for (auto pairOfBoards : frontierVisited) {

			frontier.initializeTo(pairOfBoards.first);
			visited.initializeTo(pairOfBoards.second);

			//get pieces that touch the frontier
			neighbors = frontier.getPerimeter();
			neighbors.intersectionWith(piecesExceptCurrent);


			if (problemNodes -> contains(frontier)) {

				// add problematic nodes (if any) to visited
				visited.unionWith(frontier);

				// find the problematic perimeter
				frontier = problemNodes -> getPerimeter(frontier);
				frontier.notIntersectionWith(visited);


			} else { 
				// normal search along perimeter of hive
				perimeter.floodFillStep(frontier, visited);
			}

			// get the perimeter of original neighbors
			neighbors = neighbors.getPerimeter();

			// make sure to maintain contact with any of the original neighbors
			frontier.intersectionWith(neighbors);



			if (frontier.count()) {

				for (auto node: frontier.splitIntoBitboardContainers()) {

					//store the node and the path to the node
					pair <BitboardContainer, BitboardContainer> pairOfBoards =
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
BitboardContainer MoveGenerator::getInaccessibleNodes(vector <BitboardContainer> * gates) {

	BitboardContainer inaccessible;

	for (auto gate: *gates) {
		vector <BitboardContainer> frontiers;
		vector <BitboardContainer> initialFrontiers;

		for (auto frontierMap : gate.split()) {
			for (auto board: frontierMap.second) {
				frontiers.push_back(BitboardContainer({{frontierMap.first, board}}));
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

			BitboardContainer visited;
			for (auto frontier = frontiers.begin(); frontier != frontiers.end(); frontier++) {

				if (frontier -> internalBoardCache.size() == 0) {
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
			frontiers[i].pruneCache();

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

void MoveGenerator::setGeneratingPieceBoard(BitboardContainer * b, bool pieceIsAtopHiveIn) {
	generatingPieceBoard = b;
	pieceIsAtopHive = pieceIsAtopHiveIn;
}

BitboardContainer MoveGenerator::generatePillbugSwap() {
	
	//Pillbug swap also uses lazy evaluation
	//just as beetleMove does

	BitboardContainer pillbugPerimeter = generatingPieceBoard -> getPerimeter();
	
	pillbugPerimeter.notIntersectionWith(*allPieces);

	return pillbugPerimeter;
}

//Optimize TODO
BitboardContainer MoveGenerator::getInaccessibleNodes(BitboardContainer gates) {

	vector <BitboardContainer> gatesVector = {gates};
	return getInaccessibleNodes(&gatesVector);

}
