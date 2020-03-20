#include <algorithm>
#include <set>
#include <sstream>
#include <vector>
#include <iostream>
#include <string>
#include <unordered_map>
#include "constants.h"
#include "Piece.h"
#include "Bitboard.h"
#include "PieceNode.h"
#include "Hive.h"

using namespace std;

Hive::Hive() { 
	BitboardContainer allGameBoards[] = { 
										  allPieces, whitePieces, blackPieces, ants, beetles, 
										  spiders, ladybugs, queens, mosquitoes, pillbugs, 
										  grasshoppers, gates, doors, rings, firstPieces,
										  secondPieces, thirdPieces
										};

	for (int i = 0; i < 28; ++i){
		pieceLookupTable[i] = false;
	}
}


void Hive::insertPiece(Piece piece){
	int pieceLookupNumber = pieceNum[piece.shorthandName] + 14*(piece.color == 'b');
	// if there is a hash collision
	if (pieceLookupTable[pieceLookupNumber]){
		cout << "error 0" << endl;
		throw 0;
	}
	pieceLookupTable[pieceLookupNumber]  = true;
	pieceArray[pieceLookupNumber] = piece;
	pieceLookupSet.insert(pieceLookupNumber);
}

void Hive::insertPiece(char pieceColor, string shorthandName,
					   int x, int y){		

	int pieceLookupNumber = pieceNum[shorthandName] + 14*(pieceColor == 'b');
	// If there is a hash collison
	if (pieceLookupTable[pieceLookupNumber]){
		cout << "error 8" << endl;
		throw 8;
	}
	Piece piece = Piece(pieceColor, shorthandName, x, y);
	// assign the piece to the pieceLookupTable
	pieceLookupTable[pieceLookupNumber] = true;
	pieceArray[pieceLookupNumber] = piece;
	pieceLookupSet.insert(pieceLookupNumber);
}


void Hive::print() {
	printf("not implemented yet");
}


void Hive::printDebug() {
	printf("turncounter %d \n", turnCounter);
	for (int i = 0; i < 28; ++i){
		cout << i << ":" <<  pieceLookupTable[i] << " ";
	}
	cout << endl;
	for (auto i = pieceLookupSet.begin(); i != pieceLookupSet.end(); ++i){
		Piece *piece = getPiece(*i);
		cout << *i << " " << (*piece).x << "," << (*piece).y  << endl;
		
	}
	for (int i = 0; i < 28; ++i){
		cout << i << ":" << pieceArray[i].name << endl;
	}
}



bool Hive::pieceInHive(char color, string shorthandName) {
	return pieceLookupTable[14*(color == 'b') + pieceNum[shorthandName]];
}


bool Hive::pieceInHive(int pieceNum) {
	return pieceLookupTable[pieceNum];
}

//This functionp is a mess clean it up later TODO
void Hive::movePiece(char color, string shorthandName, bool right,
					 string relativeDir, char color2, string shorthandName2){

	if (!pieceInHive(color , shorthandName) || !pieceInHive(color2, shorthandName2)){
		cout << "error 1" << endl;
		throw 1;
	}
	if (color == color2 && !shorthandName2.compare(shorthandName)){
		cout << "error 2" << endl;
		throw 2;
	}
	
	movePiece(pieceNumber(color, shorthandName), directions[relativeDir][right],
			  pieceNumber(color2, shorthandName2));
}

void Hive::movePiece(char color, string shorthandName,
		Direction dir, char color2, string shorthandName2){
	

	if (!pieceInHive(color , shorthandName) || !pieceInHive(color2, shorthandName2)){
		cout << "error 3" << endl;
		throw 3;
	}
	if (color == color2 && !shorthandName2.compare(shorthandName)){
		cout << "error 4" << endl;
		throw 4;
	}
	movePiece(pieceNumber(color, shorthandName), dir,
			  pieceNumber(color2, shorthandName2));

}

void Hive::movePiece(int piece1Int, Direction dir ,int piece2Int){
	if (!pieceInHive(piece1Int) || !pieceInHive(piece2Int)){
		cout << "error 5" << endl;
		throw 5;
	}
	if (piece1Int == piece2Int){
		cout << "error 6" << endl;
		throw 6;
	}

	int * dxdyarray = dxdy[dir];
	Piece *piece1 = getPiece(piece1Int);
	Piece *piece2 = getPiece(piece2Int);
	int x = (*piece2).x;
	int y = (*piece2).y;
	
	x += dxdyarray[0];
	y += dxdyarray[1];
	/// TODO check to make sure (x,y) doesn't have another piece on the hive already
	(*piece1).x = x;
	(*piece1).y = y;
}
/**
 * get a Piece object from those present in the current Hive
 *
 * @param pieceNumber a value between 0-27 corresponding to a possible piece in Hive
 * @return Piece object in Hive if it exists
 */
Piece * Hive::getPiece(int pieceNumber) {
	return &pieceArray[pieceNumber];	
}

Piece * Hive::getPiece(char color, string shorthandName){
	return &pieceArray[14*(color == 'b') + pieceNum[shorthandName]];
}

/**
 * takes input from stdin and converts it to an in game move
 * 
 * @param command A list of strings corresponding to space separated input
 * @return true if the game move was parsed correctly
 */
bool Hive::parseCommand(vector<string> command){
	string shorthandName = command[0].substr(1,command[0].size() );
	char color = command[0][0];

	if (turnCounter == 0){
		//if it is the first turn
		if (command.size() != 1){
			if (command[1].compare(".") != 0){
				cout << "The first move makes no sense " << endl;
				return false;
			}
		}

		if (!shorthandName.compare("Q")){
			printf("The queen bee is typically not a viable "
					"start piece under tournament rules \n");
		}

		//always start the first piece in the middle of the bitboard
		//  (0,7)                                                          
		//      (0,6)                                    ...                   
		//  (0,5)                                                          
		//      (0,4)  (1,4)          ....                                     
		//  (0,3)  (1,3)                       ....                        
		//      (0,2)  (1,2)                                                                 
		//  (0,1)  (1,1)                                                   
		//      (0,0)  
		//  (1,0)  (2,0)                                            
		// (x,y)  is the bit at the x + y*8 position 

		// board 0  board 1  board 2
		// board 3  board 4  board 5
		// board 6  board 7  board 8

		Piece firstPiece = Piece(color, shorthandName, CENTER_X, CENTER_Y);
		insertPiece(firstPiece);
		turnCounter += 1;
		return true;
	}



	if (command.size() != 2) {
		cout << "I can't figure out why there an incorrent number of words" << endl;
		return false;
	}

	char color2 = command[1][1];
	string shorthandName2 = command[1].substr(2,command[1].size());
	string relativeDir = command[1].substr(0,1);
	
	set<char> v = {'\\','/','-'};	
	bool onTheRight = (v.find(command[1][0]) == v.end());
	
	//this bool is set if "\" "-" or "/" is found to the 
	//right of the second word for example
	//
	// wB2 /bB2 -> onTheRight == false
	// bB1 wS2/ -> onTheRight == true

	if (onTheRight) {
		color2 = command[1][0];
		shorthandName2 = command[1].substr(1, command[1].size()-2);
		relativeDir = command[1].substr(command[1].size()-1, command[1].size());
	}
	Direction dir = directions[relativeDir][onTheRight];
	if (!pieceInHive(color, shorthandName)) insertPiece(color, shorthandName, -1, -1);
	movePiece(color, shorthandName, dir, color2, shorthandName2);

	//TODO make sure to be able to parse commands when one player is unable to play a move

	turnCounter += 1;	
	return true;
}

/**
 * @param color: piece color
 * @param shortHandName: a letter and number assigned to a given piece; numbers appear 
 *					     for non-unique pieces
 * @returns pieceNumber from a range [0,27] corresponding to piece type and color
 */
int Hive::pieceNumber(char color, string shorthandName) {
	return pieceNum[shorthandName] + 14*(color == 'b');
}

int Hive::pieceNumber(Piece piece){
	return Hive::pieceNumber(piece.color, piece.shorthandName);
}
set<int> Hive::getPieceLookupSet(){
	set<int> copySet(pieceLookupSet);
	return copySet; 
}

Piece * Hive::getPieceArray(){
	return pieceArray;
}
bool * Hive::getPieceLookupTable(){
	return pieceLookupTable;
}

vector <PieceNode*> Hive::getArticulationNodes() {
	vector <PieceNode*> ariticulationNodes;
}

void Hive::depthFirstSearch(){
	
}

void Hive::traverseNodes(PieceNode &n, int &counter) {
	n.visited = true;
	n.visitedNum = counter++;
	for (auto neighbor: n.neighbors) {
		if (!neighbor.visited) {
			neighbor.parent = n.visited;
			traverseNodes(neighbor, counter);	
		}
	}
}
void Hive::assignLowLink() {
}

void Hive::updateLowLink(){
}

void Hive::updateArticulationFramework(vector <PieceNode*> &affectedNodes){
}
