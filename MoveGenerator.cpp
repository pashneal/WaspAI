#include "Bitboard.h"
#include "MoveGenerator.h"
#include "constants.h"
#include <vector>
#include <unordered_map>
#include <iostream> 

using namespace std;

unordered_map< int, vector <unsigned long long>> MoveGenerator::getMoves() {	
	BitboardContainer p = allPieces->getPerimeter();
	perimeter.initializeTo(p);
	generateMoves();
	return moves.split();
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

void MoveGenerator::generateGrasshopperMoves(){
	
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
} 

void MoveGenerator::generatePillbugMoves(){
	//the Pillbug moves just as a Queen does
	generateQueenMoves();
} 

void MoveGenerator::generateMosquitoMoves(){}
void MoveGenerator::generateBeetleMoves(){}  
void MoveGenerator::generateAntMoves(){
	BitboardContainer inaccessibleNodes = getInaccessibleNodes(gatesSplit);

	//since inaccessible nodes are a subset of perimeter
	//xor them out after adding perimeter
	moves.unionWith(perimeter);
	moves.xorWith(inaccessibleNodes);
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
