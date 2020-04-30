#include "GameState.h"
#include <ctime>
#include <random>
#include <vector>
#include <set>
#include <cmath>

using namespace std;
Bitboard startSpawnBoard = Bitboard({{5, 34359738368u}});

GameState::GameState(const GameState& other) {
	*this = other;
}
void GameState::operator=( const GameState& other){
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
		possibleNames.insert(element.first);
	}
	turnColor = turnColorIn;
	unusedPieces = unusedPiecesIn;
	problemNodeContainer.allPieces = &allPieces;
	moveGenerator.allPieces = &allPieces;
	moveGenerator.problemNodes = &problemNodeContainer;

	moveGenerator.setUpperLevelPieces(&upperLevelPieces);
	moveGenerator.setStackHashTable(&stackHashTable);
}

void GameState::fastInsertPiece(Bitboard& bitboard, PieceName name) {
	if (allPieces.containsAny(bitboard)) {
		stackHashTable[bitboard.hash()].push({turnColor, name});
		upperLevelPieces.unionWith(bitboard);
	} else {
		pieceGraph.insert(bitboard);
	}

	allPieces.unionWith(bitboard);
	getPieces(name) -> unionWith(bitboard);
	getPieces(turnColor) -> unionWith(bitboard);
	immobile.initializeTo(bitboard);
	findPinnedPieces();
}

MoveInfo GameState::movePiece(Bitboard& oldBitboard, Bitboard& newBitboard,
							  PieceName name) {

	MoveInfo moveInfo;
	if (!oldBitboard.count()) 
		fastSpawnPiece(newBitboard, name);
	else
		fastMovePiece(oldBitboard, newBitboard, name);
	moveInfo.prevImmobile.initializeTo(immobile);
	moveInfo.oldPieceLocation.initializeTo(oldBitboard);
	moveInfo.pieceName = name;
	changeTurnColor();
	turnCounter++;
	return moveInfo;
}

void GameState::fastSwapPiece(Bitboard& oldBitboard, Bitboard& newBitboard,
							  PieceName name){

	PieceColor tmp = turnColor;
	turnColor = findTopPieceColor(oldBitboard);
	fastMovePiece(oldBitboard,  newBitboard, name);
	turnColor = tmp;
	changeTurnColor();
}
void GameState::fastMovePiece(Bitboard& oldBitboard, Bitboard& newBitboard,
							  PieceName name) {
	fastRemovePiece(oldBitboard, name);
	fastInsertPiece(newBitboard, name);
	changeTurnColor();
	turnCounter++;
}

void GameState::fastRemovePiece(Bitboard& oldBitboard, PieceName name){ 
	if (oldBitboard.count() == 0) {
		cout << "Attempting to remove a piece that doesn't exist" << endl;
		throw 30;
	}
	int bitHash = oldBitboard.hash();

	getPieces(name) -> xorWith(oldBitboard);
	getPieces(turnColor) -> xorWith(oldBitboard);

	// if currently in a stack
	if (stackHashTable.find(bitHash) != stackHashTable.end()) { 

		//if on top of the stack 
		if ( stackHashTable[bitHash].top().first == turnColor &&
			 stackHashTable[bitHash].top().second == name) 
		{

			stackHashTable[bitHash].pop();

			if (!(stackHashTable[bitHash].size())) {
				//if only piece in stack, remove
				stackHashTable.erase(bitHash);
				upperLevelPieces.xorWith(oldBitboard);
			}

			
			PieceName newName = findTopPieceName(oldBitboard);
			PieceColor newColor = findTopPieceColor(oldBitboard);

			//if a compenent was not found that means it was overwritten
			//by above xor operations
			if (newName == PieceName::LENGTH)
				newName = name;
			if (newColor == PieceColor::NONE)
				newColor = turnColor;

			//update allPieces with piece underneath
			getPieces(newName) ->unionWith(oldBitboard);
			getPieces(newColor) -> unionWith(oldBitboard);
		}
	} else {
		//if not in stack, remove normally
		pieceGraph.remove(oldBitboard);
		allPieces.xorWith(oldBitboard);
	}
}

void GameState::fastSpawnPiece(Bitboard& b, PieceName n) {
	int colorInt = (int)turnColor;
	unusedPieces[colorInt][n]--;
	fastInsertPiece(b, n);
	turnCounter++;
	changeTurnColor();
}

int	 GameState::countSwaps(Bitboard& piece){
	pair <Bitboard, Bitboard> swappableEmpty = getSwapSpaces(piece);
	return swappableEmpty.first.count() * swappableEmpty.second.count();
}

int GameState::countTotalUnusedPieces() {
	int colorInt = (int)turnColor;
	int totalUnusedPieces = 0;
	
	for (auto pieceAmountMap: unusedPieces[colorInt]){
		totalUnusedPieces += (bool)pieceAmountMap.second;
	}
	return totalUnusedPieces;
}
void GameState::randomSpawnPiece(Bitboard& spawnLocations) {
	int totalUnusedPieces = countTotalUnusedPieces();
	int pieceSelect = rand() % totalUnusedPieces;

	int i = 0;
	PieceName name = PieceName::LENGTH;
	for (auto pieceAmountMap: unusedPieces[(int)turnColor]){
		if (pieceAmountMap.second) i++;
		if (i - 1 == pieceSelect) {name = pieceAmountMap.first; break;}
	}

	Bitboard newPieceLocation = spawnLocations.getRandom();

	fastSpawnPiece(newPieceLocation, name);
}
void GameState::randomSwapPiece(Bitboard swappable, Bitboard empty) {
	swappable = swappable.getRandom();
	empty = empty.getRandom();
	fastSwapPiece(swappable, empty, findTopPieceName(swappable));
}

void GameState::randomMovePiece(Bitboard& initialPiece,
						  Bitboard& possibleFinalLocations, 
						  PieceName name) {

	possibleFinalLocations = possibleFinalLocations.getRandom();
	fastMovePiece(initialPiece, possibleFinalLocations, name);
}

void GameState::replayMove(MoveInfo moveInfo) {
	PieceColor oldTurnColor = turnColor;
	//if an empty move
	if (moveInfo == MoveInfo()) return;
	//if move is spawning a piece
	if(!(moveInfo.oldPieceLocation.count())) {
		//update reserve count
		unusedPieces[oldTurnColor][moveInfo.pieceName]--;
	} else {
		turnColor = findTopPieceColor(moveInfo.oldPieceLocation);
		fastRemovePiece(moveInfo.oldPieceLocation, moveInfo.pieceName);
	}
	fastInsertPiece(moveInfo.newPieceLocation, moveInfo.pieceName);
	turnCounter++;
	turnColor = oldTurnColor;
	changeTurnColor();
}
void GameState::undoMove(MoveInfo moveInfo) {
	PieceColor oldTurnColor = turnColor;
	//if an empty move
	if (moveInfo == MoveInfo() ) return;
	//if last move was spawning a piece
	if (!(moveInfo.oldPieceLocation.count())) {
		//update reserve count
		unusedPieces[oldTurnColor][moveInfo.pieceName]++;
	} else {
		turnColor = findTopPieceColor(moveInfo.oldPieceLocation);
		fastInsertPiece(moveInfo.oldPieceLocation, moveInfo.pieceName);
		turnColor = oldTurnColor;
	}
	//correct immobile piece assumption
	immobile = moveInfo.prevImmobile;

	fastRemovePiece(moveInfo.newPieceLocation, moveInfo.pieceName);
	changeTurnColor();
	turnCounter--;
}

PieceColor GameState::checkVictory() {
	//assumes no draws are in board
	Bitboard queenCheck;

	queenCheck.initializeTo(queens);
	//check white queen
	queenCheck.intersectionWith(whitePieces);
	queenCheck = queenCheck.getPerimeter();
	queenCheck.intersectionWith(allPieces);
	if (queenCheck.count() == 6) return PieceColor::BLACK;

	queenCheck.initializeTo(queens);
	//check black queen
	queenCheck.intersectionWith(blackPieces);
	queenCheck = queenCheck.getPerimeter();
	queenCheck.intersectionWith(allPieces);
	if (queenCheck.count() == 6) return PieceColor::WHITE;

	return PieceColor::NONE;
}

bool GameState::checkDraw() {
	Bitboard queenCheck(queens);
	queenCheck = queenCheck.getPerimeter();
	queenCheck.intersectionWith(allPieces);

	Bitboard originalQueenPerimeter = queens.getPerimeter();
	return (queenCheck == originalQueenPerimeter && queens.count() == 2);
}

double GameState::approximateEndResult() {
	return .5;
}

inline Bitboard * GameState::getPieces() { 
	return &allPieces;
}

Bitboard * GameState::getPieces(PieceName name) {
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
	default:
		cout << "not a valid pieceName" << endl;
		throw 5;
	}
}

Bitboard * GameState::getPieces(PieceColor color) {
	switch (color) {
		case WHITE: return &whitePieces;
		case BLACK: return &blackPieces;
		default:
			cout << "not a valid pieceColor" << endl;
			throw 6;
	}
}

void GameState::changeTurnColor() {
	if (turnColor == PieceColor::BLACK) {
		turnColor = PieceColor::WHITE;
	} else if (turnColor == PieceColor::WHITE)  {
		turnColor = PieceColor::BLACK;
	} else {
		cout << "turnColor is not valid " << turnColor  << endl;
		throw 80;
	}
}

void GameState::destroy() {
	pieceGraph.destroy();
}

//assumes game is not over
void GameState::getAllMoves() {
	pieceSpawns = getAllSpawnSpaces();	
	spawnNames = possibleNames;
	bool canMove = true;
	if (turnCounter < 2) {
		canMove = false;
		if (turnCounter == 1) {
			pieceSpawns = startSpawnBoard; 
			pieceSpawns = pieceSpawns.getPerimeter();

		} else {
			pieceSpawns = startSpawnBoard; 
		}

		spawnNames.clear();
		for (PieceName name: possibleNames) {
			if (name != PieceName::QUEEN)
				spawnNames.insert(name);
		}
	} else if ( turnCounter < 8)  {
		if (!(getPieces(turnColor)->containsAny(*getPieces(PieceName::QUEEN)))) {
			canMove = false;
			if (turnCounter >= 6) {
				spawnNames.clear();
				spawnNames.insert(PieceName::QUEEN);
			}
		}
	}

	swappableEmpty.clear(); 
	pieceMoves.clear();
	if (!canMove) return;

	Bitboard test(*getPieces(turnColor));
	test.notIntersectionWith(immobile);

	for ( Bitboard piece : test.splitIntoBitboards() ) { 
		PieceName name = findTopPieceName(piece);
		//if covered by another piece
		if (findTopPieceColor(piece) != turnColor)
			continue;
		//check for swaps if mosquito
		if (name == PieceName::MOSQUITO) {
			if (getMosquitoPillbug().containsAny(piece)) {
				swappableEmpty.push_back(getSwapSpaces(piece));	
			}
		}
		if (name == PieceName::PILLBUG ) { 
			swappableEmpty.push_back(getSwapSpaces(piece));	
		}
		if (upperLevelPieces.containsAny(piece) || !pinned.containsAny(piece)) {
			if (name == PieceName::MOSQUITO) {
				getMosquitoMoves(piece);
				continue;
			}
			moveGenerator.setGeneratingPieceBoard(&piece);
			moveGenerator.setGeneratingName(&name);
			Bitboard moves = moveGenerator.getMoves();
			pieceMoves.push_front(pair <Bitboard , Bitboard> {piece, moves});
		}
	}
}

int GameState::getAllMovesCount() {
	getAllMoves();
	int total = 0;
	for (auto iter: pieceMoves) {
		total += iter.second.count();
	}
	total += pieceSpawns.count();
	return total;
}

vector<MoveInfo> GameState::generateAllMoves() {
	vector<MoveInfo> moves;
	MoveInfo templateMove = MoveInfo();
	templateMove.prevImmobile = immobile;
	getAllMoves();
	//TODO: make findTopPieceName unneccessary;
	/* ----------------- SPAWNS --------------------------*/
	for (auto emptySquare: pieceSpawns.splitIntoBitboards()) {
		for (auto pieceAmountMap: unusedPieces[(int)turnColor]){
			if (spawnNames.find(pieceAmountMap.first) != spawnNames.end()) {
				if (pieceAmountMap.second) {
					MoveInfo newMove = templateMove;
					newMove.newPieceLocation = emptySquare;
					newMove.pieceName = pieceAmountMap.first;
					moves.push_back(newMove);
				}
			}
		}
	}
	/* ----------------- MOVES  --------------------------*/
	for (auto pieceMove: pieceMoves)  {
		for (auto move : pieceMove.second.splitIntoBitboards()) {
			MoveInfo newMove = templateMove;
			newMove.oldPieceLocation = pieceMove.first;
			newMove.pieceName = findTopPieceName(pieceMove.first);
			newMove.newPieceLocation = move;
			moves.push_back(newMove);
		}
	}
	
	/* ----------------- SWAPS  --------------------------*/
	for (auto se: swappableEmpty) {
		for (auto swappable: se.first.splitIntoBitboards()) {
			for (auto empty: se.second.splitIntoBitboards()) {
				MoveInfo newMove = templateMove;
				newMove.oldPieceLocation = swappable;
				newMove.newPieceLocation = empty;
				newMove.pieceName = findTopPieceName(swappable);
				moves.push_back(newMove);
			}
		}
	}
	return moves;
}

Bitboard GameState::getMosquitoMoves(Bitboard piece) {
	Bitboard moves, generated;
	PieceName beetle = PieceName::BEETLE;
	moveGenerator.setGeneratingName(&beetle);
	moveGenerator.setGeneratingPieceBoard(&piece);

	//if on top of the hive
	if (upperLevelPieces.containsAny(piece)) {
		//can only move like a beetle on top of the hive
		moves = moveGenerator.getMoves();
		return moves;
	}	

	Bitboard piecePerimeter = piece.getPerimeter();
	Bitboard testUpperLevel(upperLevelPieces);

	//get all upperLevelPieces adjecent to this one and on top of hive
	testUpperLevel.intersectionWith(piecePerimeter);

	for (Bitboard test: testUpperLevel.splitIntoBitboards()) {
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

Bitboard GameState::getMosquitoPillbug() {
	Bitboard mosquitoPillbug = *getPieces(PieceName::PILLBUG);
	mosquitoPillbug.notIntersectionWith(upperLevelPieces);
	mosquitoPillbug = mosquitoPillbug.getPerimeter();
	mosquitoPillbug.intersectionWith(*getPieces(PieceName::MOSQUITO));

	//make sure mosquito is not apart of a stack of pieces
	if (stackHashTable.find(mosquitoPillbug.hash()) != stackHashTable.end()) 
		mosquitoPillbug.clear();
	return mosquitoPillbug;
}

Bitboard GameState::getAllSpawnSpaces() {
	Bitboard oppositePiecePerimeter;
	Bitboard spawns = allPieces.getPerimeter();

	if (turnColor == PieceColor::WHITE) {
		oppositePiecePerimeter = blackPieces;
	} else {
		oppositePiecePerimeter = whitePieces;
	}


	//convert pieces to top most colors 
	for (auto piece: upperLevelPieces.splitIntoBitboards()) {
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
bool GameState::makePsuedoRandomMove() {
	if (turnCounter < 8) {
			return makeTrueRandomMove();
	}
	Bitboard notCovered(allPieces);
	//remove covered pieces 
	notCovered.notIntersectionWith(upperLevelPieces);
	//remove any piece that might have been swapped
	notCovered.notIntersectionWith(immobile);
	//can only move pieces of correct color
	notCovered.intersectionWith(*getPieces(turnColor));

	vector<movesCollection> movesPerPiece((int)PieceName::LENGTH);

	int total = 0;
	int prevTotal;

	Bitboard test;

	//add a slot for each piece found
	for (auto name : possibleNames) {

		test.initializeTo(*getPieces(name));
		test.intersectionWith(notCovered); 

		if (test.count()) {
			for (Bitboard piece: test.splitIntoBitboards() ) {
				int numMoves = 1;
				//bool isPinned = pinned.containsAny(piece);
				//int numMoves = moveApproximation(piece, name, isPinned);
				//if (numMoves == 0) continue;
				movesPerPiece[name].push_back(make_pair(piece, numMoves));
				total += numMoves;
				prevTotal = total;
			}
		}
	}

	//deal with upperLevelPieces
	for (Bitboard piece: upperLevelPieces.splitIntoBitboards()) {
		if (stackHashTable[piece.hash()].top().first == turnColor) {
			PieceName name = stackHashTable[piece.hash()].top().second;

			// a piece that is on the upperLevel is not pinned
			//int numMoves = moveApproximation(piece, PieceName::BEETLE, false);
			int numMoves = 1;
			total += numMoves;
			movesPerPiece[name].push_back(make_pair(piece, numMoves));
		}
	}

	bool madeAMove = true;
	if(!attemptSpawn(total)) {
		if (!attemptMove(movesPerPiece, total)) {
			//if all move approximations were incorrect
			//attempt another spawn
			madeAMove = attemptSpawn(0);
		}
	}

	if (madeAMove) return true;
	//if no move was made, pass a turn
	changeTurnColor();
	return false;
}

bool GameState::makeTrueRandomMove() {
	getAllMoves();
	int total = 0;
	for (auto pieceMove: pieceMoves) {
		total += pieceMove.second.count();
	}
	for (auto se: swappableEmpty) {
		total += se.first.count() * se.second.count();
	}


	int colorInt = (int)turnColor;
	int totalUnusedPieces = 0;
	
	for (auto pieceAmountMap: unusedPieces[colorInt]){
		if (spawnNames.find(pieceAmountMap.first) != spawnNames.end())
			totalUnusedPieces += (bool)pieceAmountMap.second;
	}

	total += pieceSpawns.count() * totalUnusedPieces;

	if (!total) {changeTurnColor(); return false;}
	total = dist(e2) % total;

	int moveSelect =0 ;
	for (auto pieceMove: pieceMoves) {
		moveSelect += pieceMove.second.count();
		if (moveSelect > total) {
			Bitboard random = pieceMove.second.getRandom();
			fastMovePiece(pieceMove.first, random, findTopPieceName(pieceMove.first));
			return true;
		}
	}

	for (auto se: swappableEmpty) {
		moveSelect += se.first.count() * se.second.count();
		if (moveSelect > total) {
			Bitboard random = se.first.getRandom();
			Bitboard randomEmpty = se.second.getRandom();
			fastSwapPiece(random, randomEmpty, findTopPieceName(random));
			return true;
		}
	}

	Bitboard random = pieceSpawns.getRandom();
	int pieceSelect = rand() % totalUnusedPieces;

	int i = 0;
	PieceName name = PieceName::LENGTH;
	for (auto pieceAmountMap: unusedPieces[(int)turnColor]){
		if (spawnNames.find(pieceAmountMap.first) != spawnNames.end()) {
			if (pieceAmountMap.second) i++;
			if (i - 1 == pieceSelect) {name = pieceAmountMap.first; break;}
		}
	}

	fastSpawnPiece(random, name);
	return true;
}
bool GameState::attemptSpawn(int totalApproxMoves) {
	Bitboard spawns = getAllSpawnSpaces();
	int spawnsCount = countTotalUnusedPieces();
	if (spawns.count() == 0) return false;

	//if there are no legal spawns 
	if(spawnsCount == 0) return false;
	int randInt = rand() % (totalApproxMoves + spawnsCount );

	if (randInt >= totalApproxMoves) {
		randomSpawnPiece(spawns);
		return true;
	}
	return false;
}	

bool GameState::attemptMove(vector<movesCollection>& approxMovesPerPiece, int total){
	while (total) {

		int random = rand() % total;
		int approxMoveSelect = 0;
		int approxMoveCount = 0;
		movesCollection::iterator boardNumMoves;
		Bitboard pieceBoard;

		PieceName name;
		bool flag = false;

		//count number of moves for every piece 
		for (unsigned int i = 0; i < approxMovesPerPiece.size() ; i++){
			boardNumMoves = approxMovesPerPiece[i].begin();
			while (boardNumMoves != approxMovesPerPiece[i].end()) {
				approxMoveSelect += boardNumMoves->second;

				if  (approxMoveSelect > random) {
					pieceBoard = boardNumMoves->first;
					approxMoveCount = boardNumMoves->second;
					name = (PieceName)i;
					flag = true;
					break;
				}
				boardNumMoves++;
			}
			if (flag) break;
		}
		
		Bitboard moves;
	
		
		//generate moves if not pinned
		if (!pinned.containsAny(pieceBoard)) { 
			if (name == PieceName::MOSQUITO) {
				moves = getMosquitoMoves(pieceBoard);
			} else {
				moveGenerator.setGeneratingName(&name);
				moveGenerator.setGeneratingPieceBoard(&pieceBoard);
				moves =  moveGenerator.getMoves();
			}
		}

		int movesCount = moves.count();

		//if a pillbug or a mosquito-pillbug
		if ( (name == PieceName::MOSQUITO && getMosquitoPillbug().containsAny(pieceBoard) ) 
			 || name == PieceName::PILLBUG) {

			int swapsCount = countSwaps(pieceBoard);
			if (swapsCount) {
				//reroll to account for swaps
				int randInt = rand() % (swapsCount + movesCount);

				if (randInt >= movesCount){
					auto swappableEmpty = getSwapSpaces(pieceBoard);
					randomSwapPiece(swappableEmpty.first, swappableEmpty.second);
					return true;
				}
			}
		}
		if (movesCount) {
			randomMovePiece(pieceBoard, moves, name);
			return true;
		}

		//move approximation is incorrect so update
		total -= approxMoveCount;
		approxMovesPerPiece[name].erase(boardNumMoves);
	}
	//no move was made
	return false;
}	

int GameState::moveApproximation(Bitboard piece, PieceName name, bool isPinned){
	if (isPinned && name != PieceName::PILLBUG) return 0;
	switch (name) {
		case MOSQUITO:
		{
			Bitboard piecePerimeter = piece.getPerimeter();

			//get places in perimeter where pieces exist
			piecePerimeter.intersectionWith(allPieces);

			//inherits no moves from adjacent mosquito
			piecePerimeter.notIntersectionWith(*getPieces(PieceName::MOSQUITO));
			
			//if pinned, only inherits moves from adjacent pillbugs
			if (isPinned) 
				piecePerimeter.intersectionWith(*getPieces(PieceName::PILLBUG));

			int approxMoves = 0;
			//inherits moves of surrounding pieces
			for (auto adjPiece: piecePerimeter.splitIntoBitboards()) {
				approxMoves += moveApproximation(piece, findTopPieceName(adjPiece), isPinned);
			}


			return approxMoves;
		}
		case QUEEN:
		{
			return 2;
		}
		case GRASSHOPPER:
		{
			//can jump over a piece that is beside them
			piece = piece.getPerimeter();
			return piece.count();
		}
		case LADYBUG:
		{
			//Ladybug is afforded more moves if she can climb more pieces
			Bitboard visited;
			allPieces.floodFillStep(piece, visited);
			allPieces.floodFillStep(piece, visited);
			return (int)(1.5)*visited.count();
		}
		case BEETLE: 
		{
			// if the beetle is on the hive it has more freedom
			return 4 + 2*(upperLevelPieces.containsAny(piece)); 
		}
		case ANT:
		{
			// The ant can usually go almost anywhere in the perimeter 
			// But most of the ant moves are terrible
			// Increase for more random results
			int max = .1*sqrt(allPieces.getPerimeter().count());
			cout << allPieces.getPerimeter().count() << endl;
			if (max == 0) max = 1;
			return max;
		}
		case SPIDER:
		{
			//the spider will almost always have two moves
			return 2;
		}
		case PILLBUG:
		{
			piece = piece.getPerimeter();
			Bitboard noPiece(piece);
			noPiece.notIntersectionWith(allPieces);
			piece.intersectionWith(allPieces);

			//can usually move two squares and can usually swap out
			// all but one piece into an empty square
			return (2 + noPiece.count() * (piece.count() -1));
		}
		default:
			cout << "cannot find the approximateMove for given PieceName" << endl;
			throw 7;
		}
}

PieceName GameState::findTopPieceName(Bitboard piece) {
	if (upperLevelPieces.containsAny(piece)) {
		return stackHashTable[piece.hash()].top().second; 
	}
	for (auto name: possibleNames) {
		if (getPieces(name) -> containsAny(piece)) return name;
	}

	return PieceName::LENGTH;
}

//only finds the colors of lower level pieces
PieceColor GameState::findTopPieceColor( Bitboard piece) {
	if (upperLevelPieces.containsAny(piece)) {
		return stackHashTable[piece.hash()].top().first; 
	}
	for (int i = 0 ; i < 2; i ++ )  {
		if (getPieces((PieceColor)i) -> containsAny(piece) ) 
			return	(PieceColor)i;
	}
	return PieceColor::NONE;
}

inline void GameState::findPinnedPieces(){
	pinned = pieceGraph.getPinnedPieces();
}

//returns pair <swappable, empty>
//where any one piece from swappable can be moved to empty
pair <Bitboard, Bitboard> GameState::getSwapSpaces(Bitboard piece) {
	Bitboard swappable = piece.getPerimeter();
	Bitboard empty = piece.getPerimeter();

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
	Bitboard legal = moveGenerator.getPillbugSwapSpaces();

	swappable.intersectionWith(legal);
	empty.intersectionWith(legal);

	return pair <Bitboard, Bitboard> {swappable, empty};
}

//returns a negative number if an error occured
int GameState::playout(int limitMoves) {
	for (int i = 0; i < limitMoves; i++) {
		if (limitMoves == 0)
			return i;
		if (checkDraw())
			return i;
		if (checkVictory() != PieceColor::NONE)
			return i;
		//in case both sides cannot move (very rare)
		if (!makePsuedoRandomMove()) 
				if (!makePsuedoRandomMove()) 
					return i;

	}
	return limitMoves;
}

void GameState::print() {
	cout << "turnColor" << endl; 
	cout << turnColor << endl;			
	cout << "turnCounter" << endl; 
	cout << turnCounter << endl;		
	cout << "allPieces" << endl; 
	allPieces.print(); 
	cout << "whitePieces" << endl; 
	whitePieces.print();       
	cout << "blackPieces" << endl; 
	blackPieces.print();       
	cout << "ants" << endl; 
	ants.print();              
	cout << "beetles" << endl; 
	beetles.print();           
	cout << "spiders" << endl; 
	spiders.print();           
	cout << "ladybugs" << endl; 
	ladybugs.print();          
	cout << "queens" << endl; 
	queens.print();            
	cout << "mosquitoes" << endl; 
	mosquitoes.print();        
	cout << "pillbugs" << endl; 
	pillbugs.print();          
	cout << "grasshoppers" << endl; 
	grasshoppers.print();      
	cout << "upperLevelPieces" << endl; 
	upperLevelPieces.print();  
	cout << "immobile" << endl; 
	immobile.print();          
	cout << "unused pieces" << endl;
	cout << "player 0" << endl;
	for (auto iter: unusedPieces[0])
		cout << "pieceName " << iter.first << " amountRemaining " << iter.second << endl;
	cout << "player 1" << endl;
	for (auto iter: unusedPieces[1])
		cout << "pieceName " << iter.first << " amountRemaining " << iter.second << endl;
}
//STILL WORKING ON RANDOM 
//FORGOT THE RULE ABOUT PILLBUG CAN BE USED IF PINNED
//QUEEN CAN'T BE PLACED FIRST (FOR NOW)
//NO OTHER PIECE CAN MOVE IF QUEEN NOT PLACED

//TODO: delayed legality check

//recenter / overflow?
//end evaluation func
