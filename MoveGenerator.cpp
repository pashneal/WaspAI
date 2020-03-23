#include "Bitboard.h"
#include "MoveGenerator.h"
#include "constants.h"
#include <vector>
#include <unordered_map>
#include <iostream> 

using namespace std;

BitboardContainer MoveGenerator::getMoves() {	
	BitboardContainer piecesExceptCurrent = *allPieces;

	//remove generatingPiece
	piecesExceptCurrent.xorWith(*generatingPieceBoard);

	BitboardContainer p = piecesExceptCurrent.getPerimeter();
	perimeter.initializeTo(p);
	generateMoves();
	return moves;
}

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

void MoveGenerator::setGatesSplit( vector <BitboardContainer> * p){
	gatesSplit = p;
}

//TODO:optimize it is so slow
void MoveGenerator::generateGrasshopperMoves(){

	for (Direction dir: gameDirections){
		BitboardContainer testBitboard, nextPiece;

		nextPiece.initializeTo(testBitboard);

		nextPiece.shiftDirection(dir);
		
		//check if there is a piece to jump over
		nextPiece.intersectionWith(*allPieces);
		if (nextPiece.internalBoardCache.size() == 0) {
			//cannot move in this Direction
			//if no piece to jump over
			continue;
		}

		bool pieceExists;
		do{
			//see if there is a piece to jump over
			nextPiece.shiftDirection(dir);
			pieceExists = allPieces -> containsAny(nextPiece);

		// end only there is no more piece
		} while (pieceExists);

		moves.unionWith(nextPiece);
	}

}

void MoveGenerator::generateQueenMoves(){ 
	BitboardContainer frontier, visited;
	frontier.initializeTo(*generatingPieceBoard);

	//Find a valid move (if any) in the perimeter
	perimeter.floodFillStep(frontier, visited);

	//Get edges that lie along gates
	BitboardContainer inaccessibleFrontier;
	visited.initializeTo(*generatingPieceBoard);
	gatesCombined -> floodFillStep(inaccessibleFrontier, visited);

	//remove inaccessible nodes and initial location
	frontier.xorWith(inaccessibleFrontier);
	frontier.unionWith(*generatingPieceBoard);
	frontier.xorWith(*generatingPieceBoard);

	moves.unionWith(frontier);
}

void MoveGenerator::generateLadybugMoves(){
	
	BitboardContainer frontier, visited;
	frontier.initializeTo(*generatingPieceBoard);

	//must first make two moves on top of the hive
	allPieces -> floodFillStep(frontier, visited);
	//remove nodes from an earlier depth
	frontier.unionWith(visited);
	frontier.xorWith(visited);
	allPieces -> floodFillStep(frontier, visited);
	frontier.unionWith(visited);
	frontier.xorWith(visited);

	//must make a move along the perimiter
	perimeter.floodFillStep(frontier, visited);

	//remove nodes from an earlier depth
	frontier.xorWith(visited);

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
	//Beetle generation is lazy
	//Further analysis is necessary in order to see
	//If moves atop the hive are legal
	BitboardContainer traversable;

	traversable.initializeTo(perimeter);
	traversable.unionWith(*allPieces);
	BitboardContainer * temp = allPieces;

	allPieces = &traversable;

	generateQueenMoves();

	allPieces = temp;
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

	BitboardContainer frontier, visited, emptyVisited, inaccessible;
	frontier.initializeTo(*generatingPieceBoard);

	for (int i = 0; i < NUM_SPIDER_MOVES; i++){

		emptyVisited.clear();
		
		//get every edge that is disallowed from current nodes
		inaccessible.initializeTo(frontier);
		BitboardContainer inaccessiblePerimeter = inaccessible.getPerimeter();
		inaccessiblePerimeter.intersectionWith(*gatesCombined);

		//reinitialize to nodes that are illegal if reached from current node
		inaccessible.initializeTo(inaccessiblePerimeter);
		inaccessible.pruneCache();

		if (!inaccessible.internalBoardCache.size()){
			//remove all nodes that touch illegal edges
			frontier.unionWith(*gatesCombined);
			frontier.xorWith(*gatesCombined);

			//if there exists some other way to access a gate
			//set inaccessible to it
			frontier.floodFillStep(inaccessible, emptyVisited);

			frontier.xorWith(inaccessible);
			frontier.pruneCache();
		}
		//expand search along board perimeter
		perimeter.floodFillStep(frontier, visited);

		//only keep the newest nodes
		frontier.unionWith(visited);
		frontier.xorWith(visited);
	}
	moves.unionWith(frontier);
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
		unsigned long searchResolvedCount;
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

//Optimize TODO
BitboardContainer MoveGenerator::getInaccessibleNodes(BitboardContainer gates) {
	vector <BitboardContainer> gatesVector = {gates};
	return getInaccessibleNodes(&gatesVector);
}
