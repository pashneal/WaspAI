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
}

void MoveGenerator::setAllGates( vector <BitboardContainer> * p){
	allGates = p;
}

void MoveGenerator::generateGrasshopperMoves(){
	
}
void MoveGenerator::generateQueenMoves(){}   
void MoveGenerator::generateLadybugMoves(){} 
void MoveGenerator::generatePillbugMoves(){} 
void MoveGenerator::generateMosquitoMoves(){}
void MoveGenerator::generateBeetleMoves(){}  
void MoveGenerator::generateAntMoves(){
}     
void MoveGenerator::generateSpiderMoves(){}  

BitboardContainer MoveGenerator::getInaccessibleNodes() {
	for (auto gate: *allGates) {
		vector <BitboardContainer> frontiers;
		for (auto frontierMap : gate.split()) {
			for (auto board: frontierMap.second) {
				frontiers.push_back(BitboardContainer({{frontierMap.first, board}}));
			}
		}


		while (1) {
			BitboardContainer visited;
			for (auto frontier = frontiers.begin(); frontier != frontiers.end(); frontier++) {

				visited.initializeTo(*frontier);

				//conduct a search and store prev nodes in visited	
				perimeter.floodFillStep(*frontier, visited);

				//TODO: replace with andNot()
				//delete gate nodes from the frontier
				frontier -> unionWith(gate);
				frontier -> xorWith(gate);

				//remove empty boards
				frontier -> pruneCache();

				if (frontier -> internalBoardCache.size() == 0) break;
			}
		}
	}
}

