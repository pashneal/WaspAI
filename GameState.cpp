#include "GameState.h"
#include <ctime>
#include <random>
#include <vector>
#include <set>

using namespace std;

GameState::GameState (list <PieceName> possibleNamesIn) {
	possibleNames = possibleNamesIn;
	problemNodeContainer.allPieces = &allPieces;
	moveGenerator.allPieces = &allPieces;
	moveGenerator.problemNodes = &problemNodeContainer;
}

MoveInfo GameState::insertPiece(BitboardContainer& bitboard, PieceName& name) {
	MoveInfo moveInfo;
	moveInfo.pieceName = name;
	if (bitboard.count() == 0) return moveInfo;
	fastInsertPiece(bitboard, name);
	moveInfo.newPieceLocation.initializeTo(bitboard);
	return moveInfo;
}

void GameState::fastInsertPiece(BitboardContainer& bitboard, PieceName& name) {
	pieceGraph.insert(bitboard);
	problemNodeContainer.insertPiece(bitboard);

	if (allPieces.containsAny(bitboard)) {
		stackHashTable[bitboard.hash()].push({turnColor, name});
		upperLevelPieces.xorWith(bitboard);
	}

	allPieces.unionWith(bitboard);
	getPieces(name) -> unionWith(bitboard);
	getPieces(turnColor) -> unionWith(bitboard);
	immobile.initializeTo(bitboard);
	
}

MoveInfo GameState::movePiece(BitboardContainer& oldBitboard, BitboardContainer& newBitboard,
							  PieceName& name) {

	MoveInfo moveInfo = insertPiece(newBitboard, name);
	fastMovePiece(oldBitboard, newBitboard, name);
	moveInfo.oldPieceLocation.initializeTo(oldBitboard);
	return moveInfo;
}

void GameState::fastMovePiece(BitboardContainer& oldBitboard, BitboardContainer& newBitboard,
							  PieceName& name) {

	fastInsertPiece(newBitboard, name);
	fastRemovePiece(oldBitboard, name);
}

void GameState::fastRemovePiece(BitboardContainer& oldBitboard, PieceName& name){ 
	int bitHash = oldBitboard.hash();
	if (stackHashTable.find(bitHash) != stackHashTable.end()) { 
		if (stackHashTable[bitHash].top().first == turnColor &&
		    stackHashTable[bitHash].top().second == name) 
		{
			stackHashTable[bitHash].pop();
			
			if (!(stackHashTable[bitHash].size())) {
				stackHashTable.erase(bitHash);
				upperLevelPieces.xorWith(oldBitboard);
			}
		}
	} else {
		problemNodeContainer.removePiece(oldBitboard);
		pieceGraph.remove(oldBitboard);
		allPieces.xorWith(oldBitboard);
		getPieces(name) -> xorWith(oldBitboard);
		getPieces(turnColor) -> xorWith(oldBitboard);
	}
}

void GameState::undoMove(MoveInfo moveInfo) {
	changeTurnColor();
	movePiece(moveInfo.newPieceLocation, moveInfo.oldPieceLocation,
			  moveInfo.pieceName);
}

PieceColor GameState::checkVictory() {
	//assumes no draws are in board
	BitboardContainer queenCheck;

	queenCheck.initializeTo(queens);
	//check white queen
	queenCheck.intersectionWith(whitePieces);
	queenCheck = queenCheck.getPerimeter();
	queenCheck.intersectionWith(allPieces);
	if (queenCheck.count() == 6) return PieceColor::BLACK;

	queenCheck.initializeTo(queens);
	//check black queen
	queenCheck.intersectionWith(whitePieces);
	queenCheck = queenCheck.getPerimeter();
	queenCheck.intersectionWith(allPieces);
	if (queenCheck.count() == 6) return PieceColor::WHITE;

	return PieceColor::NONE;
}

bool GameState::checkDraw() {
	//TODO: draw by repetition OUTSIDE of GameState
	BitboardContainer queenCheck(queens);
	queenCheck = queenCheck.getPerimeter();
	queenCheck.intersectionWith(allPieces);

	BitboardContainer originalQueenPerimeter = queens.getPerimeter();
	return (queenCheck == originalQueenPerimeter);
}

inline BitboardContainer * GameState::getPieces() { 
	return &allPieces;
}

BitboardContainer * GameState::getPieces(PieceName name) {
	switch (name)   
	{
	case GRASSHOPPER: return &grasshoppers ;
	case QUEEN:       return &queens       ;
	case LADYBUG:     return &ladybugs     ;
	case PILLBUG:     return &pillbugs     ;
	case MOSQUITO:    return &mosquitoes   ;
	case BEETLE:      return &beetles      ;
	case ANT:         return &ants         ;
	case SPIDER:      return &spiders      ;
		break;
	}
}

BitboardContainer * GameState::getPieces(PieceColor color) {
	switch (color) {
		case WHITE: return &whitePieces;
		case BLACK: return &blackPieces;
		case NONE:  return &allPieces;
	}
}

void GameState::changeTurnColor() {
	if (turnColor == PieceColor::BLACK) {
		turnColor = PieceColor::WHITE;

	} else  {
		turnColor = PieceColor::BLACK;
	}
}

void GameState::destroy() {
	pieceGraph.destroy();
}

//assumes game is not over
void GameState::getAllMoves(list <PieceName> names) {
	if (turnCounter < 2) {
		return;
	}

	if (turnCounter < 8)  {
		BitboardContainer test(*getPieces(PieceName::QUEEN));
		test.intersectionWith(*getPieces(turnColor));
		if (!test.count()) return;
	}

	BitboardContainer test;
	for (PieceName name: names) {
		moveGenerator.setGeneratingName(&name);

		//get turnColor pieces of a certain name
		test.initializeTo(*getPieces(name));
		test.intersectionWith(*getPieces(turnColor));	

		for ( BitboardContainer piece : test.splitIntoBitboardContainers() ) { 

			if (name == PieceName::MOSQUITO) {
				getMosquitoMoves(piece);
				continue;
			}

			moveGenerator.setGeneratingPieceBoard(&piece);
			BitboardContainer moves = moveGenerator.getMoves();
			pieceMoves.push_front(pair <BitboardContainer , BitboardContainer> {piece, moves});
		}
	}
}

void GameState::getMosquitoMoves(BitboardContainer piece) {
	BitboardContainer moves;
	PieceName beetle = PieceName::BEETLE;
	moveGenerator.setGeneratingName(&beetle);
	moveGenerator.setGeneratingPieceBoard(&piece);

	//if on top of the hive
	if (upperLevelPieces.containsAny(piece)) {
		//can only move like a beetle on top of the hive
		BitboardContainer moves = moveGenerator.getMoves();
		pieceMoves.push_front(pair <BitboardContainer , BitboardContainer> {piece, moves});
		return;
	}	

	BitboardContainer piecePerimeter = piece.getPerimeter();
	BitboardContainer testUpperLevel = upperLevelPieces;

	//get all upperLevelPieces adjecent to this one and on top of hive
	testUpperLevel.intersectionWith(piecePerimeter);

	for (BitboardContainer test: testUpperLevel.splitIntoBitboardContainers()) {
		if(stackHashTable[test.hash()].top().second == PieceName::BEETLE) {

			moves = moveGenerator.getMoves();
			pieceMoves.push_front(pair <BitboardContainer , BitboardContainer> {piece, moves});

		}
	}

	//removes nodes resolved by above
	piecePerimeter.notIntersectionWith(testUpperLevel);

	for (PieceName name: possibleNames) {
		//see if mosquito is beside one of these types
		if (piecePerimeter.containsAny(*getPieces(name))) {
			moveGenerator.setGeneratingName(&name);
			moves = moveGenerator.getMoves();
			pieceMoves.push_front(pair <BitboardContainer , BitboardContainer> {piece, moves});
		}
	}
}

BitboardContainer GameState::getAllSpawnSpaces() {
	BitboardContainer oppositePiecePerimeter;
	BitboardContainer spawns = allPieces.getPerimeter();

	if (turnColor == PieceColor::WHITE) {
		oppositePiecePerimeter = blackPieces;
	} else {
		oppositePiecePerimeter = whitePieces;
	}


	//convert pieces to top most colors 
	for (auto piece: upperLevelPieces.splitIntoBitboardContainers()) {
		if (stackHashTable[piece.hash()].top().first == turnColor) {
			oppositePiecePerimeter.notIntersectionWith(piece);
		}
	}
	oppositePiecePerimeter = oppositePiecePerimeter.getPerimeter();
	spawns.notIntersectionWith(oppositePiecePerimeter);
	return spawns;
}

//faster than random
//but does not store in move information
void GameState::makePsuedoRandomMove() {
	BitboardContainer notPinned(allPieces);

	//find all non-pinned pieces
	notPinned.notIntersectionWith(pinned);
	
	//remove covered pieces as well
	notPinned.notIntersectionWith(upperLevelPieces);

	//remove any piece that might have been swapped
	notPinned.notIntersectionWith(immobile);

	list <PieceName> names;
	list <int> upperBound;
	list <BitboardContainer> boards;

	int total = 0;
	BitboardContainer test;
	
	int numMoves;
	//add a slot for each piece found
	for (auto name : possibleNames) {
			
		test.initializeTo(notPinned);
		test.intersectionWith(*getPieces(name));
		if (test.count()) {
			for (BitboardContainer piece: test.splitIntoBitboardContainers() ) {
				if (name == PieceName::MOSQUITO) {
					getMosquitoMoves(piece);
					numMoves = pieceMoves.front().second.count();
					pieceMoves.clear();
				} else {
					numMoves = moveApproximation(piece, name);
				}
				total += numMoves;
				upperBound.push_front(total);
				names.push_front(name);
				boards.push_front(piece);
			}
		}
	}

	//deal with upperLevelPieces
	
	for (BitboardContainer piece: upperLevelPieces.splitIntoBitboardContainers()) {
		if (stackHashTable[piece.hash()].top().first == turnColor) {
			numMoves = moveApproximation(piece, PieceName::BEETLE);
			total += numMoves;
			upperBound.push_front(total);
			names.push_front(stackHashTable[piece.hash()].top().second);
			boards.push_front(piece);
		}
	}

	//get mosquitoes that touch pillbugs
	BitboardContainer mosquitoPillbug = getPieces(PieceName::PILLBUG)->getPerimeter();
	mosquitoPillbug.intersectionWith(*getPieces(PieceName::MOSQUITO));

	//make sure mosquito is not apart of a stack of pieces
	if (stackHashTable.find(mosquitoPillbug.hash()) != stackHashTable.end()) 
		mosquitoPillbug.clear();
		

	//If mosquito-pillbug count swaps
	if (mosquitoPillbug.count() == 1) {
		test.initializeTo(mosquitoPillbug);
		test = test.getPerimeter();

		//get movable pieces
		test.intersectionWith(notPinned);
		numMoves = test.count();


		//find empty spaces beside mosquito
		test.initializeTo(mosquitoPillbug);
		test = test.getPerimeter();
		test.notIntersectionWith(allPieces);

		//multiply by movable pieces
		numMoves *= test.count();
		total += numMoves;
	}
	
		srand(time(NULL));
		int randInt = rand() % total;

		// if the randInt is greater than first of list,
		// perform a swap
		if (randInt > upperBound.front() ) {
			pair <BitboardContainer, BitboardContainer> swappableEmpty =
				getSwapSpaces(mosquitoPillbug);
			int moveNumber = upperBound.front();
			for (auto swappable : swappableEmpty.first.splitIntoBitboardContainers()) {
				for (auto empty : swappableEmpty.second.splitIntoBitboardContainers() ) {
					if (moveNumber == randInt) {

						//perform a swap
						PieceName name = findPieceName(swappable);
						fastMovePiece(swappable,empty,name);
						return;
					}
					moveNumber++;
				}
			}
		}

		auto iterNames = names.begin();
		auto iterUpperBound = upperBound.begin();
		auto iterBoards = boards.begin();
		while (iterNames != names.end() ) {
			if (*iterUpperBound < randInt) {
				//if randInt is too large
				iterNames++; iterUpperBound++; iterBoards++;
			} else { 
				PieceName name = *iterNames;
				BitboardContainer initialPiece = *iterBoards;
				//IF PILLBUG (SWAPS)
				//IF MOSQUITO (MOVES)
				moveGenerator.setGeneratingName(&name);
				moveGenerator.setGeneratingPieceBoard(&initialPiece);


				BitboardContainer moves = moveGenerator.getMoves();


			
				//reinitialize random to be more accurate
				randInt = rand() % moves.count();
				//select and perform a move according to random
				int moveSelect = 0;
				for (BitboardContainer piece: moves.splitIntoBitboardContainers() ) {
					if (moveSelect == randInt) {
						fastMovePiece(initialPiece, piece, name);
						return;
					}
					moveSelect++;
				}

				
			}
		}
}	

int GameState::moveApproximation(BitboardContainer piece, PieceName name){
	switch (name) {
		case MOSQUITO:
			//hell if I know
			return -1000;
		case QUEEN:
			return 2;
		case GRASSHOPPER:
			//highly variable but expensive to compute (currently)
			return 2;
		case LADYBUG:
			//ladybug is relatively inexpensive to compute and highly variable
			moveGenerator.setGeneratingName(&name);
			moveGenerator.setGeneratingPieceBoard(&piece);
		case BEETLE:
			// if the beetle is on the hive it has more freedom
			return 4 + 2*(upperLevelPieces.containsAny(piece)); 
		case ANT:
			// The ant can usually go almost anywhere in the perimeter 
			return (int) (allPieces.getPerimeter().count()*.9);
		case SPIDER:
			//the spider will almost always have two moves
			return 2;
		case PILLBUG:
			piece = piece.getPerimeter();
			BitboardContainer noPiece(piece);
			noPiece.notIntersectionWith(allPieces);
			piece.intersectionWith(allPieces);

			//can usually move two squares and can usually swap out
			// all but one piece into an empty square
			return (2 + noPiece.count() * (piece.count() -1));
		}
}

PieceName GameState::findPieceName(BitboardContainer piece) {
	BitboardContainer test;
	for (auto name: possibleNames) {
		test.initializeTo(piece);
		test.intersectionWith(*getPieces(name));
		if (test.count()) return name;
	}
	return PieceName::QUEEN;
}

void GameState::findPinnedPieces(){
	pinned = pieceGraph.getPinnedPieces();
}

//returns pair <swappable, empty>
//where any one piece from swappable can be moved to empty
pair <BitboardContainer, BitboardContainer> GameState::getSwapSpaces(BitboardContainer piece) {
	BitboardContainer swappable = piece.getPerimeter();
	BitboardContainer empty = piece.getPerimeter();

	swappable.intersectionWith(allPieces);
	//can't swap pinned pieces
	swappable.notIntersectionWith(pinned);
	//can't swap anything that moved last turn
	swappable.notIntersectionWith(immobile);
	//cant swap upperLevelPieces
	swappable.notIntersectionWith(upperLevelPieces);

	//allowed to swap if no piece on space
	empty.notIntersectionWith(allPieces);

	return pair <BitboardContainer, BitboardContainer> {swappable, empty};
}
//STILL WORKING ON RANDOM 
//FORGOT THE RULE ABOUT PILLBUG CAN BE USED IF PINNED
//QUEEN CAN'T BE PLACED FIRST (FOR NOW)
//NO OTHER PIECE CAN MOVE IF QUEEN NOT PLACED

//no piece can move if it was moved last turn
//(that includes swaps)

//Pillbug swap and last move restriction;
//cannot swap out from up top hive or through gates
//blocked when covered
//can still use power if pinned

//beetle also can't move through upperLevelGates

//Mosquito movement ={
//only a bettle on the hive
//also can't move through upperLevel gates

//delayed legality check
