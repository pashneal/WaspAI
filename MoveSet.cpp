#include "Bitboard.h"
#include "MoveSet.h"
#include "constants.h"
#include <vector>
#include <unordered_map>
#include <iostream> 

using namespace std;

MoveSet::MoveSet(PieceName pieceN , BitboardContainer pieces, BitboardContainer gates) {
	pieceName = pieceN;
	allPieces = pieces;
	allGates = gates;
	generateMoves();
	perimeter = allPieces.getPerimeter();
}

unordered_map< int, vector <unsigned long long>> MoveSet::getMoves() {	
	return moves.split();
}



void MoveSet::generateMoves() {
	switch(pieceName){
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


void MoveSet::generateGrasshopperMoves(){

}
void MoveSet::generateQueenMoves(){}   
void MoveSet::generateLadybugMoves(){} 
void MoveSet::generatePillbugMoves(){} 
void MoveSet::generateMosquitoMoves(){}
void MoveSet::generateBeetleMoves(){}  
void MoveSet::generateAntMoves(){
}     
void MoveSet::generateSpiderMoves(){}  
