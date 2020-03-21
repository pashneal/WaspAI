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

//TODO: Optimize so you don't have to iterate through every gate separately 
//Note: the spider cannot reach more than 3 squares away as an optimization
BitboardContainer MoveGenerator::getInaccessibleNodes() {
	BitboardContainer inaccessible;
	for (auto gate: *allGates) {
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

