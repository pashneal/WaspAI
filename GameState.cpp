#include "GameState.h"
#include <ctime>
#include <random>
#include <vector>
#include <set>

using namespace std;

//TEST
GameState::GameState( GameState& other) {
	turnColor =			other.turnColor;
	turnCounter =		other.turnCounter;
	allPieces =         other.allPieces;
	whitePieces =       other.whitePieces;
	blackPieces =       other.blackPieces;
	ants =              other.ants;
	beetles =           other.beetles;
	spiders =           other.spiders;
	ladybugs =          other.ladybugs;
	queens =            other.queens;
	mosquitoes =        other.mosquitoes;
	pillbugs =          other.pillbugs;
	grasshoppers =      other.grasshoppers;
	upperLevelPieces =  other.upperLevelPieces;
	immobile =          other.immobile;
	pinned =            other.pinned;          

	unusedPieces=       other.unusedPieces;
	pieceMoves=         other.pieceMoves;
	possibleNames=      other.possibleNames;
	stackHashTable=     other.stackHashTable;     

	problemNodeContainer = other.problemNodeContainer;
	problemNodeContainer.allPieces = &allPieces;

	pieceGraph = other.pieceGraph;
	moveGenerator.allPieces = &allPieces;
	moveGenerator.problemNodes = &problemNodeContainer;

	moveGenerator.setUpperLevelPieces(&upperLevelPieces);
	moveGenerator.setStackHashTable(&stackHashTable);
}

GameState::GameState (vector <unordered_map <PieceName, int>> unusedPiecesIn, 
					  PieceColor turnColorIn) {
	for (auto element: unusedPiecesIn[0]) {
		possibleNames.push_front(element.first);
	}
	turnColor = turnColorIn;
	unusedPieces = unusedPiecesIn;
	problemNodeContainer.allPieces = &allPieces;
	moveGenerator.allPieces = &allPieces;
	moveGenerator.problemNodes = &problemNodeContainer;

	moveGenerator.setUpperLevelPieces(&upperLevelPieces);
	moveGenerator.setStackHashTable(&stackHashTable);
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
	if (allPieces.containsAny(bitboard)) {
		stackHashTable[bitboard.hash()].push({turnColor, name});
		upperLevelPieces.unionWith(bitboard);
	} else {
		pieceGraph.insert(bitboard);
		problemNodeContainer.insertPiece(bitboard);
	}

	allPieces.unionWith(bitboard);
	getPieces(name) -> unionWith(bitboard);
	getPieces(turnColor) -> unionWith(bitboard);
	immobile.initializeTo(bitboard);
	findPinnedPieces();
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
	changeTurnColor();
	turnCounter++;
}

void GameState::fastRemovePiece(BitboardContainer& oldBitboard, PieceName& name){ 
	if (oldBitboard.count() == 0) {
		cout << "Attempting to remove a piece that doesn't exist" << endl;
		throw 30;
	}
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
	}
	getPieces(name) -> xorWith(oldBitboard);
	getPieces(turnColor) -> xorWith(oldBitboard);
}

void GameState::fastSpawnPiece(BitboardContainer& b, PieceName& n) {
	int colorInt = (int)turnColor;
	unusedPieces[colorInt][n]--;
	fastInsertPiece(b, n);
	turnCounter++;
	changeTurnColor();
}

int	 GameState::countSwaps(BitboardContainer& piece){
	pair <BitboardContainer, BitboardContainer> swappableEmpty = getSwapSpaces(piece);
	return swappableEmpty.first.count() * swappableEmpty.second.count();
}

int GameState::countPossibleSpawns(BitboardContainer& spawns) {
	int colorInt = (int)turnColor;
	int totalUnusedPieces = 0;
	for (auto pieceAmountMap: unusedPieces[colorInt]){
		totalUnusedPieces += (bool)pieceAmountMap.second;
	}
	return totalUnusedPieces*spawns.count();
}
void GameState::spawnPiece(BitboardContainer& spawnLocations, int moveSelect) {
	int colorInt = (int)turnColor;
	int i = moveSelect;
	for (auto pieceAmountMap: unusedPieces[colorInt]){
		for (int j = 0; j < pieceAmountMap.second; j++) {
			if ( i  == 0) {
				i = moveSelect % spawnLocations.count();
				for (auto spawnLocation: spawnLocations.splitIntoBitboardContainers() ) {
					
					if (i == 0) {
						PieceName name = pieceAmountMap.first;
						fastSpawnPiece(spawnLocation, name);
						return;
					}
					i--;
				}

			}
			i = i/spawnLocations.count();
		}
	}
}
void GameState::swapPiece(BitboardContainer& swappable, BitboardContainer& empty, int moveSelect) {
	int i = 0;
	for (auto initialPiece: swappable.splitIntoBitboardContainers() )  {
		for (auto finalPiece: empty.splitIntoBitboardContainers() ) {
			if (moveSelect  == i) {
				PieceName name = findPieceName(initialPiece);
				PieceColor temp = turnColor; turnColor = findPieceColor(initialPiece);
				fastMovePiece(initialPiece, finalPiece, name);	
				turnColor = temp;
				changeTurnColor();
				return;
			}
			i++;
		}
	}
}

void GameState::movePiece(BitboardContainer& initialPiece,
						  BitboardContainer& possibleFinalLocations,
						  int moveSelect) {
	int i = 0;
	for (BitboardContainer piece: possibleFinalLocations.splitIntoBitboardContainers() ) {
		if (moveSelect == i) {
			PieceName name = findPieceName(initialPiece);
			fastMovePiece(initialPiece, piece, name);
			return;
		}
		i++;
	}
}

void GameState::undoMove(MoveInfo moveInfo) {

	//TODO: ADD LOGIC TO UNDO A SPAWN
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
	return (queenCheck == originalQueenPerimeter && queens.count() == 2);
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

BitboardContainer GameState::getMosquitoMoves(BitboardContainer piece) {
	BitboardContainer moves, generated;
	PieceName beetle = PieceName::BEETLE;
	moveGenerator.setGeneratingName(&beetle);
	moveGenerator.setGeneratingPieceBoard(&piece);

	//if on top of the hive
	if (upperLevelPieces.containsAny(piece)) {
		//can only move like a beetle on top of the hive
		moves = moveGenerator.getMoves();
		return moves;
	}	

	BitboardContainer piecePerimeter = piece.getPerimeter();
	BitboardContainer testUpperLevel = upperLevelPieces;

	//get all upperLevelPieces adjecent to this one and on top of hive
	testUpperLevel.intersectionWith(piecePerimeter);

	for (BitboardContainer test: testUpperLevel.splitIntoBitboardContainers()) {
		if(stackHashTable[test.hash()].top().second == PieceName::BEETLE) {
			generated = moveGenerator.getMoves();
			moves.unionWith(generated);
		}
	}

	//removes nodes resolved by above loop
	piecePerimeter.notIntersectionWith(testUpperLevel);

	for (PieceName name: possibleNames) {
		//see if mosquito is beside one of these types
		if (piecePerimeter.containsAny(*getPieces(name))) {
			moveGenerator.setGeneratingName(&name);
			generated = moveGenerator.getMoves();
			moves.unionWith(generated);
		}
	}
	return moves;
}

BitboardContainer GameState::getMosquitoPillbug() {
	BitboardContainer mosquitoPillbug = *getPieces(PieceName::PILLBUG);
	mosquitoPillbug.notIntersectionWith(upperLevelPieces);
	mosquitoPillbug = mosquitoPillbug.getPerimeter();
	mosquitoPillbug.intersectionWith(*getPieces(PieceName::MOSQUITO));

	//make sure mosquito is not apart of a stack of pieces
	if (stackHashTable.find(mosquitoPillbug.hash()) != stackHashTable.end()) 
		mosquitoPillbug.clear();
	return mosquitoPillbug;
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
	BitboardContainer notCovered(allPieces);

	//remove covered pieces 
	notCovered.notIntersectionWith(upperLevelPieces);

	//remove any piece that might have been swapped
	notCovered.notIntersectionWith(immobile);

	//can only move pieces of correct color
	notCovered.intersectionWith(*getPieces(turnColor));

	list <PieceName> names;
	list <int> upperBound;
	list <BitboardContainer> boards;

	int total = 0;
	BitboardContainer test;
	BitboardContainer mosquitoPillbug = getMosquitoPillbug();

	int numMoves;
	//add a slot for each piece found
	for (auto name : possibleNames) {

		test.initializeTo(*getPieces(name));
		
		if (name == PieceName::MOSQUITO) {
			numMoves = getMosquitoMoves(test).count();

			//if mosquito is touching a pillbug
			//count swaps as part of move
			if (mosquitoPillbug.count())
				numMoves += countSwaps(test);	

			total+= numMoves;
			upperBound.push_front(total);
			names.push_front(name);
			boards.push_front(test);
			continue;
		}

		test.intersectionWith(notCovered);
		if (test.count()) {
			for (BitboardContainer piece: test.splitIntoBitboardContainers() ) {

				total += moveApproximation(piece, name);
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

	auto iterNames = names.begin();
	auto iterUpperBound = upperBound.begin();
	auto iterBoards = boards.begin();

	BitboardContainer spawns = getAllSpawnSpaces();
	int spawnsCount = countPossibleSpawns(spawns);
	total +=  spawnsCount;

	//if there are no legal moves
	if (total == 0) return;
	srand(time(NULL));
	int randInt = rand() % total;

	if (randInt >= total - spawnsCount ) 
		spawnPiece(spawns, rand() % spawnsCount );

	
	while (iterNames != names.end() ) {
		if (*iterUpperBound < randInt) {
			//if randInt is too large
			iterNames++; iterUpperBound++; iterBoards++;
			continue;
		} 
		PieceName name = *iterNames;
		BitboardContainer initialPiece = *iterBoards;
		moveGenerator.setGeneratingName(&name);
		moveGenerator.setGeneratingPieceBoard(&initialPiece);
		BitboardContainer moves;
		int moveCount = 0;
		int swapCount = 0;
		if (notCovered.containsAny(initialPiece)) { 
			moves = moveGenerator.getMoves();
			moveCount += moves.count();
		}

		switch(name) {
			case PILLBUG:
				//TODO: refactor duplicate code
				{

					pair <BitboardContainer, BitboardContainer> swappableEmpty =
						getSwapSpaces(initialPiece);

					swapCount += swappableEmpty.first.count() * swappableEmpty.second.count();

					randInt = rand() % (moveCount + swapCount);
					if (randInt < moveCount){
						movePiece(initialPiece, moves, randInt);
					} else {
						swapPiece(swappableEmpty.first, 
								  swappableEmpty.second,
								  randInt - moveCount);
					}
					return;
				}
			case MOSQUITO:
				{

					if (mosquitoPillbug.count()) {
						pair <BitboardContainer, BitboardContainer> swappableEmpty =
							getSwapSpaces(initialPiece);

						swapCount += swappableEmpty.first.count() * swappableEmpty.second.count();

						randInt = rand() % (moveCount + swapCount);

						if (randInt >= moveCount){
							swapPiece(swappableEmpty.first, 
									swappableEmpty.second,
									randInt - moveCount);
							return;
						}
					}
					movePiece(initialPiece, moves, randInt);
					return;
				}
			default:
				randInt = rand() % moves.count();
				movePiece(initialPiece, moves, randInt);
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
			//highly variable but expensive to compute
			piece = piece.getPerimeter();
			piece.intersectionWith(allPieces);
			return (int)(2.5)*piece.count();
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

//only find the name of lower level pieces
PieceName GameState::findPieceName(BitboardContainer piece) {
	for (auto name: possibleNames) 
		if (getPieces(name) -> containsAny(piece)) return name;

	return PieceName::QUEEN;
}

//only finds the colors of lower level pieces
PieceColor GameState::findPieceColor( BitboardContainer piece) {
	for (int i = 0 ; i < 2; i ++ ) 
		if (getPieces((PieceColor)i) -> containsAny(piece) ) 
			return	(PieceColor) i;

	return PieceColor::NONE;
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

	moveGenerator.setGeneratingPieceBoard(&piece);
	BitboardContainer legal = moveGenerator.getPillbugSwapSpaces();

	swappable.intersectionWith(legal);
	empty.intersectionWith(legal);

	return pair <BitboardContainer, BitboardContainer> {swappable, empty};
}

void GameState::playout(int limitMoves) {
	if (limitMoves == 0)
		return;
	if (checkVictory() != PieceColor::NONE)
		return;
	if (checkDraw())
		return;
	makePsuedoRandomMove();
	limitMoves--;
}

//given a maximum allowed piece count for each piece
//count every piece in the GameState and setUnusedPieces accordingly
bool GameState::setUnusedPieces(vector <unordered_map <PieceName, int>> maxPieceCount) {
	bool flag = false;
	for (int i = 0; i < 2; i++) {
		PieceColor color = (PieceColor)i;
		for (PieceName name: possibleNames) {
			BitboardContainer uncoveredPieces(*getPieces(color));
			uncoveredPieces.intersectionWith(*getPieces(name));
			uncoveredPieces.notIntersectionWith(upperLevelPieces);

			int numPieces = maxPieceCount[color][name] - uncoveredPieces.count();

			BitboardContainer coveredPieces(*getPieces(color));
			coveredPieces.intersectionWith(*getPieces(name));
			coveredPieces.intersectionWith(upperLevelPieces);

			for (BitboardContainer s: coveredPieces.splitIntoBitboardContainers() ) {
				stack < pair < PieceColor, PieceName > > stackCopy = stackHashTable[s.hash()];

				while (!stackCopy.empty()) {
					pair <PieceColor, PieceName> p = stackCopy.top();
					if (p.first == color && p.second == name) 
						numPieces--;
					stackCopy.pop();
				}
			}

			if (numPieces < 0 ) {
				cout << "greater than allowed number of Pieces in GameState" << endl;
				cout << "PieceColor " << color;
				cout << "PieceName " << name;
				throw 40;
			}

			flag |= unusedPieces[color][name] != numPieces;
			unusedPieces[color][name] = numPieces;
		}
	}
	return flag;
}
//STILL WORKING ON RANDOM 
//FORGOT THE RULE ABOUT PILLBUG CAN BE USED IF PINNED
//QUEEN CAN'T BE PLACED FIRST (FOR NOW)
//NO OTHER PIECE CAN MOVE IF QUEEN NOT PLACED

//TODO: delayed legality check

//recenter / overflow?
//end evaluation func
