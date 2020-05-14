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
	pieceStacks=        other.pieceStacks;     

	problemNodeContainer = other.problemNodeContainer;
	problemNodeContainer.allPieces = &allPieces;

	pieceGraph = other.pieceGraph;
	moveGenerator.allPieces = &allPieces;
	moveGenerator.problemNodes = &problemNodeContainer;

	moveGenerator.setUpperLevelPieces(&upperLevelPieces);
	moveGenerator.setPieceStacks(&pieceStacks);
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
	moveGenerator.setPieceStacks(&pieceStacks);
}

void GameState::fastInsertPiece(Bitboard& bitboard, PieceName name, PieceColor color) {
	if (allPieces.containsAny(bitboard)) {
		upperLevelPieces.unionWith(bitboard);
	} else {
		pieceGraph.insert(bitboard);
	}
	pieceStacks[bitboard.hash()].push_front({color, name});
	allPieces.unionWith(bitboard);
	getPieces(name) -> unionWith(bitboard);
	getPieces(color) -> unionWith(bitboard);
	immobile.initializeTo(bitboard);
	findPinnedPieces();
}

MoveInfo GameState::movePiece(Bitboard& oldBitboard, Bitboard& newBitboard,
							  PieceName name) { 
	MoveInfo moveInfo;
	if (!oldBitboard.count()) 
		fastSpawnPiece(newBitboard, name);
	else
		fastMovePiece(oldBitboard, newBitboard);
	moveInfo.prevImmobile.initializeTo(immobile);
	moveInfo.oldPieceLocation.initializeTo(oldBitboard);
	moveInfo.pieceName = name;
	changeTurnColor();
	turnCounter++;
	return moveInfo;
}

void GameState::fastMovePiece(Bitboard& oldBitboard, Bitboard& newBitboard) {
	PieceName name = findTopPieceName(oldBitboard);
	PieceColor color = findTopPieceColor(oldBitboard);
	fastRemovePiece(oldBitboard);
	fastInsertPiece(newBitboard, name, color);
	changeTurnColor();
	turnCounter++;
}

void GameState::fastRemovePiece(Bitboard& oldBitboard){ 
	if (oldBitboard.count() == 0) {
		cout << "Attempting to remove a piece that doesn't exist" << endl;
		throw 30;
	}
	int bitHash = oldBitboard.hash();

	PieceColor color = findTopPieceColor(oldBitboard); 
	PieceName name = findTopPieceName(oldBitboard); 
	//remove oldBitboard and assume nopiece is underneath
	getPieces(name) -> xorWith(oldBitboard);
	getPieces(color) -> xorWith(oldBitboard);
	pieceStacks[bitHash].pop_front();

	if (pieceStacks[bitHash].size() == 0) {
		//if was only piece in stack, remove completely
		pieceStacks.erase(bitHash);
		pieceGraph.remove(oldBitboard);
		allPieces.xorWith(oldBitboard);
	} else {
		
		if (pieceStacks[bitHash].size()==1 ){
			//remove from upperLevelPieces
			upperLevelPieces.notIntersectionWith(oldBitboard);
		}

		if (pieceStacks[bitHash].size() > 1){
			//update gameState with stack underneath
			auto copy = pieceStacks[bitHash];
			while (copy.size()){
				getPieces(copy.front().first) -> unionWith(oldBitboard);
				getPieces(copy.front().second) -> unionWith(oldBitboard);
				copy.pop_front();
			}
		} else {
			//update gameState with single piece underneath
			PieceColor newColor = pieceStacks[bitHash].front().first;
			getPieces(newColor) -> unionWith(oldBitboard);
			PieceName name = pieceStacks[bitHash].front().second;
			getPieces(name) -> unionWith(oldBitboard);
		}
	}
}

void GameState::fastSpawnPiece(Bitboard& b, PieceName n) {
	int colorInt = (int)turnColor;
	unusedPieces[colorInt][n]--;
	fastInsertPiece(b, n, turnColor);
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
	fastMovePiece(swappable, empty);
}

void GameState::randomMovePiece(Bitboard& initialPiece,
						  Bitboard& possibleFinalLocations){
	possibleFinalLocations = possibleFinalLocations.getRandom();
	fastMovePiece(initialPiece, possibleFinalLocations);
}
void GameState::replayMove(MoveInfo moveInfo) {
	//if an empty move
	if (moveInfo == MoveInfo()) {
		turnCounter++; changeTurnColor(); immobile.clear(); return;
	}

	PieceColor color  = turnColor;
	//if move is spawning a piece
	if(!(moveInfo.oldPieceLocation.count())) {
		//update reserve count
		unusedPieces[turnColor][moveInfo.pieceName]--;
	} else {
		color = findTopPieceColor(moveInfo.oldPieceLocation);
		fastRemovePiece(moveInfo.oldPieceLocation);
	}
	fastInsertPiece(moveInfo.newPieceLocation, moveInfo.pieceName, color);
	turnCounter++; changeTurnColor();
}
void GameState::undoMove(MoveInfo moveInfo) {
	if (moveInfo == MoveInfo()) {
		cout << "Not allowed to undo an empty move" << endl;
		cout << "prevImmobile value needs to be set" << endl;
		throw 4;
	}

	PieceColor color  = turnColor;
	//if last move was spawning a piece
	if (!(moveInfo.oldPieceLocation.count())) {
		//update reserve count
		unusedPieces[turnColor][moveInfo.pieceName]++;
	} else {
		// find color of existing piece
		color = findTopPieceColor(moveInfo.newPieceLocation);
		fastInsertPiece(moveInfo.oldPieceLocation, moveInfo.pieceName, color);
	}

	//correct immobile piece assumption
	immobile = moveInfo.prevImmobile;
	fastRemovePiece(moveInfo.newPieceLocation);
	if (!moveInfo.oldPieceLocation.count())
		findPinnedPieces();
}

PieceColor GameState::checkVictory() {
	Bitboard whiteQueen, blackQueen;
	//find the respective queens
	for (Bitboard& queen: queens.splitIntoBitboards()){
		auto& queenStack = pieceStacks[queen.hash()];
		if (queenStack.back().first == PieceColor::WHITE) {
			whiteQueen = queen;
		} else {
			blackQueen = queen;
		}
	}


	//if the blackQueen is surrounded, white wins!
	blackQueen = blackQueen.getPerimeter();
	blackQueen.intersectionWith(allPieces);
	if (blackQueen.count() == 6) return PieceColor::WHITE;

	//if the whiteQueen is surrounded, black wins!
	whiteQueen = whiteQueen.getPerimeter();
	whiteQueen.intersectionWith(allPieces);
	if (whiteQueen.count() == 6) return PieceColor::BLACK;
	
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
	
	int parameters[2][9];
	
	for (int i = 0 ; i < 2 ; i++ ) {
		int surroundCount;
		int enemyCount = 0;
		int mobileFriendly = 0;
		int pillbugKillShotControl = 0;
		int pillbugFreeSquares = 0;
		int pillbugEscapeSquares = 0;
		int unpinnedCount = 0;
		int moveCount = 0;
		bool queenCanMove = false;
		PieceColor color = (PieceColor)i;
		PieceName name;

		turnColor = color;
		moveCount = generateAllMoves().size();

		//get queen
		Bitboard queen;
		for (auto testQueen : queens.splitIntoBitboards()){
			if (findBottomPieceColor(testQueen) == color){
				queen = testQueen;
			}
		}


		if (!queen.count()) {
			print();
			cout << "Queens are doing wonky things" << endl;
			throw 91;
		}

		//count number of pieces surrounding queen
		Bitboard surrounding(queen);
		surrounding = surrounding.getPerimeter();
		surrounding.intersectionWith(allPieces);
		surroundCount = surrounding.count();

		//count mobile and friendly pieces
		surrounding.intersectionWith(*getPieces(color));
		surrounding.notIntersectionWith(pinned);
		surrounding.notIntersectionWith(upperLevelPieces);
		surrounding.notIntersectionWith(immobile);
		for (auto& piece: surrounding.splitIntoBitboards()) {
			name = findTopPieceName(piece);
			moveGenerator.setGeneratingName(&name);
			moveGenerator.setGeneratingPieceBoard(&piece);
			mobileFriendly += (moveGenerator.getMoves().count() > 0);
		} 

		//count enemy upperLevelPieces near Queen
		Bitboard nearQueen(queen);	
		for (int i = 0 ; i < 3; i++){
			Bitboard temp = nearQueen.getPerimeter();
			nearQueen.unionWith(temp);
		}
		Bitboard enemies(nearQueen);
		enemies.intersectionWith(upperLevelPieces);

		for (auto& enemy: enemies.splitIntoBitboards()){
			enemyCount += (findTopPieceColor(enemy) != color);
		}

		//see whether a nearby friendly pillbug is can still use its power
		Bitboard activePillbug(*getPieces(color));
		activePillbug.intersectionWith(nearQueen);
		activePillbug.intersectionWith(pillbugs);
		activePillbug.notIntersectionWith(upperLevelPieces);
		activePillbug.notIntersectionWith(immobile);

		if (activePillbug.count()){
			//count squares around queen the pillbug can swap pieces away from
			Bitboard pillbugControl = activePillbug.getPerimeter();
			Bitboard killshots = queen.getPerimeter();
			pillbugControl.notIntersectionWith(immobile);
			killshots.intersectionWith(pillbugControl);
			pillbugKillShotControl = killshots.count();
			cout << pillbugKillShotControl;

			//see how many squares around pillbug are empty
			pillbugControl.notIntersectionWith(allPieces);
			pillbugEscapeSquares = pillbugControl.count();

			//same as above but don't include killshots
			pillbugControl.notIntersectionWith(killshots);
			pillbugFreeSquares = pillbugControl.count();

		}

		name = PieceName::QUEEN;
		if (!pinned.containsAny(queen) && !upperLevelPieces.containsAny(queen)){
			moveGenerator.setGeneratingName(&name);
			moveGenerator.setGeneratingPieceBoard(&queen);
			queenCanMove = moveGenerator.getMoves().count();
		};

		//count unpinned pieces
		Bitboard unpinned(*getPieces(color));
		unpinned.notIntersectionWith(upperLevelPieces);
		unpinned.notIntersectionWith(pinned);
		unpinnedCount = unpinned.count();


		parameters[color][0] = surroundCount;
		parameters[color][1] = enemyCount;
		parameters[color][2] = mobileFriendly;
		parameters[color][3] = unpinnedCount;
		parameters[color][4] = queenCanMove; 
		parameters[color][5] = pillbugEscapeSquares;
		parameters[color][6] = moveCount;
		parameters[color][7] = pillbugFreeSquares;
		parameters[color][8] = pillbugKillShotControl;
	}
	double weights[7] = {-.025, -.025, -.01, .03, .2, .02, .01};
	double score = 0;
	for (int i = 0 ; i < 7; i++){
		score += weights[i]*parameters[WHITE][i] - weights[i]*parameters[BLACK][i];
	}
	score += parameters[WHITE][8]*.033*parameters[WHITE][7] 
			- parameters[BLACK][8]*.033*parameters[BLACK][7];
	score = std::min(.5, std::max( -.5, score));
	return score;
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
	/*----------------------------SPAWNS--------------------------------*/
	//if the first two moves have yet to be made, alter
	//normal spawning rules
	if (turnCounter < 2) {
		canMove = false;
		if (turnCounter == 1) {
			pieceSpawns = startSpawnBoard; 
			pieceSpawns = pieceSpawns.getPerimeter();

		} else {
			pieceSpawns = startSpawnBoard; 
		}

		//Hive tournament rule: 
		//queen is not allowed to spawn first
		spawnNames.clear();
		for (PieceName name: possibleNames) {
			if (name != PieceName::QUEEN)
				spawnNames.insert(name);
		}

	} else if ( turnCounter < 8)  {
		//Hive rule:
		//If queen is not yet placed by the player's fourth turn
		//force a queen placement
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

	for ( Bitboard& piece : test.splitIntoBitboards() ) { 
		/*----------------------------SWAPS--------------------------------*/
		PieceName name = findTopPieceName(piece);
		//if covered by unfriendly piece, this piece cannot move
		if (findTopPieceColor(piece) != turnColor)
			continue;
		//check for swaps if mosquito
		if (name == PieceName::MOSQUITO) {
			if (getMosquitoPillbug().containsAny(piece)) {
				swappableEmpty.push_back(getSwapSpaces(piece));	
			}
		}
		//check for swaps if pillbug
		if (name == PieceName::PILLBUG ) { 
			swappableEmpty.push_back(getSwapSpaces(piece));	
		}

		/*----------------------------MOVES--------------------------------*/
		if (upperLevelPieces.containsAny(piece) || !pinned.containsAny(piece)) {
			if (name == PieceName::MOSQUITO) {
				pieceMoves.push_back(std::make_pair(piece, getMosquitoMoves(piece)));
				continue;
			}
			moveGenerator.setGeneratingPieceBoard(&piece);
			moveGenerator.setGeneratingName(&name);
			Bitboard moves = moveGenerator.getMoves();

			pieceMoves.push_front(std::make_pair(piece, moves));
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
	for (auto& emptySquare: pieceSpawns.splitIntoBitboards()) {
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
		for (auto& move : pieceMove.second.splitIntoBitboards()) {
			MoveInfo newMove = templateMove;
			newMove.oldPieceLocation = pieceMove.first;
			newMove.pieceName = findTopPieceName(pieceMove.first);
			newMove.newPieceLocation = move;
			moves.push_back(newMove);
		}
	}
	/* ----------------- SWAPS  --------------------------*/
	for (auto se: swappableEmpty) {
		for (auto& swappable: se.first.splitIntoBitboards()) {
			for (auto& empty: se.second.splitIntoBitboards()) {
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
		//Hive rule:
		//can only move like a beetle on top of the hive
		moves = moveGenerator.getMoves();
		return moves;
	}	

	Bitboard piecePerimeter = piece.getPerimeter();
	Bitboard testUpperLevel(upperLevelPieces);

	//get all upperLevelPieces adjecent to this one and on top of hive
	testUpperLevel.intersectionWith(piecePerimeter);

	for (Bitboard& test: testUpperLevel.splitIntoBitboards()) {
		if(pieceStacks[test.hash()].front().second == PieceName::BEETLE) {
			generated = moveGenerator.getMoves();
			moves.unionWith(generated);
		}
	}

	//removes nodes resolved by above loop
	piecePerimeter.notIntersectionWith(testUpperLevel);

	for (PieceName name: possibleNames) {
		//see if mosquito is adjecent to one of these types
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
	if (upperLevelPieces.containsAny(mosquitoPillbug)) 
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
	for (auto& piece: upperLevelPieces.splitIntoBitboards()) {
		if (pieceStacks[piece.hash()].front().first == turnColor) {
			oppositePiecePerimeter.notIntersectionWith(piece);
		}
	}
	oppositePiecePerimeter = oppositePiecePerimeter.getPerimeter();
	spawns.notIntersectionWith(oppositePiecePerimeter);
	return spawns;
}

//faster than random
//but does not store nor return move information
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

	Bitboard test;

	//add a slot for each piece found
	for (auto name : possibleNames) {
		test.initializeTo(*getPieces(name));
		test.intersectionWith(notCovered); 

		if (test.count()) {
			for (Bitboard& piece: test.splitIntoBitboards() ) {
				//int numMoves = 1;
				bool isPinned = !pinned.containsAny(piece);
				int numMoves = moveApproximation(piece, isPinned);
				if (numMoves == 0) continue;
				movesPerPiece[name].push_back(make_pair(piece, numMoves));
				total += numMoves;
			}
		}
	}

	//deal with upperLevelPieces
	for (Bitboard& piece: upperLevelPieces.splitIntoBitboards()) {
		if (pieceStacks[piece.hash()].front().first == turnColor) {
			PieceName name = pieceStacks[piece.hash()].front().second;

			int numMoves = moveApproximation(piece, false);
			//int numMoves = 1;
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
	turnCounter++;
	changeTurnColor();
	immobile.clear();
	return false;
}

bool GameState::makeTrueRandomMove() {

	getAllMoves();
	int total = 0;
	//count moves, swaps, and spawns respectively
	for (auto pieceMove: pieceMoves) {
		total += pieceMove.second.count();
	}
	for (auto se: swappableEmpty) {
		total += se.first.count() * se.second.count();
	}
	int totalUnusedPieces = 0;
	for (auto pieceAmountMap: unusedPieces[turnColor]){
		if (spawnNames.find(pieceAmountMap.first) != spawnNames.end())
			totalUnusedPieces += (bool)pieceAmountMap.second;
	}
	total += pieceSpawns.count() * totalUnusedPieces;

	//if no moves available, make empty move
	if (!total) {changeTurnColor(); return false;}
	
	total = dist(e2) % total;

	/*----------------------------MOVES-----------------------------*/
	int moveSelect =0 ;
	for (auto pieceMove: pieceMoves) {
		moveSelect += pieceMove.second.count();
		if (moveSelect > total) {
			Bitboard random = pieceMove.second.getRandom();
			fastMovePiece(pieceMove.first, random);
			return true;
		}
	}

	/*----------------------------SWAPS-----------------------------*/
	for (auto se: swappableEmpty) {
		moveSelect += se.first.count() * se.second.count();
		if (moveSelect > total) {
			Bitboard random = se.first.getRandom();
			Bitboard randomEmpty = se.second.getRandom();
			fastMovePiece(random, randomEmpty);
			return true;
		}
	}

	/*----------------------------SPAWNS-----------------------------*/
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
	int spawnsCount = countTotalUnusedPieces()*spawns.count();
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

				//select a random move to operation on
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
		
		//generate moves if not pinned or 
		//if on top of another piece
		if (!pinned.containsAny(pieceBoard) || upperLevelPieces.containsAny(pieceBoard)) { 
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
		//if no swap made, attempt to make a move
		if (movesCount) {
			randomMovePiece(pieceBoard, moves);
			return true;
		}

		//no swap or move was made
		//move approximation was incorrect so update
		total -= approxMoveCount;
		approxMovesPerPiece[name].erase(boardNumMoves);
	}
	//no swap or move was made
	return false;
}	

int GameState::moveApproximation(Bitboard piece, bool isPinned){
	PieceName name = findTopPieceName(piece);
	if (isPinned && name != PieceName::PILLBUG && !upperLevelPieces.containsAny(piece)) return 0;
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
			for (auto& adjPiece: piecePerimeter.splitIntoBitboards()) {
				approxMoves += moveApproximation(adjPiece, isPinned);
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
			int max = (int)(.5*allPieces.getPerimeter().count());
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
	return pieceStacks.at(piece.hash()).front().second; 
}

PieceColor GameState::findTopPieceColor( Bitboard piece) {
	return pieceStacks.at(piece.hash()).front().first; 
}

PieceName GameState::findBottomPieceName(Bitboard piece) {
	return pieceStacks.at(piece.hash()).back().second; 
}

PieceColor GameState::findBottomPieceColor(Bitboard piece) {
	return pieceStacks.at(piece.hash()).back().first; 
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

int GameState::playout(int limitMoves) {
	for (int i = 0; i < limitMoves; i++) {
		if (limitMoves == 0)
			return i;
		if (checkDraw())
			return i;
		if (checkVictory() != PieceColor::NONE)
			return i;
		//in case both sides cannot move (very rare)
		//I'm not sure it is even possible
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
