#include "Arena.h"

unordered_map <string, PieceColor> colorNotationReverse {
	{"w", PieceColor::WHITE},
	{"b", PieceColor::BLACK},
};
unordered_map <PieceColor, string> colorNotation {
	{PieceColor::WHITE, "w"},
	{PieceColor::BLACK, "b"},
};
unordered_map <char, vector<Direction>> dirNotationReverse{
	{'\\', {Direction::SE,Direction::NW}},
	{'-',  {Direction::NE,Direction::SW}},
	{'/',  {Direction::E, Direction::W }}
};
unordered_map <Direction, string> dirNotation {
	{Direction::NW ,"\\"},
	{Direction::NE ,"/"},
	{Direction::SW ,"/"},
	{Direction::SE ,"\\"},
	{Direction::W  ,"-"},
	{Direction::E  ,"-"},
};

unordered_map<string, PieceName> nameNotationReverse{ 
	{"G", PieceName::GRASSHOPPER},
	{"Q",		 PieceName::QUEEN},
	{"L", 	 PieceName::LADYBUG},
	{"P", 	 PieceName::PILLBUG},	
	{"M",	 PieceName::MOSQUITO},
	{"B",  	 PieceName::BEETLE},
	{"A",     	 PieceName::ANT},
	{"S",  	 PieceName::SPIDER},
};

unordered_map<PieceName, string> nameNotation{ 
	{PieceName::GRASSHOPPER, "G"},
	{PieceName::QUEEN,		 "Q"},
	{PieceName::LADYBUG, 	 "L"},
	{PieceName::PILLBUG, 	 "P"},	
	{PieceName::MOSQUITO,	 "M"},
	{PieceName::BEETLE,  	 "B"},
	{PieceName::ANT,     	 "A"},
	{PieceName::SPIDER,  	 "S"},
};

set<PieceName> singlePieces = {MOSQUITO, QUEEN, LADYBUG, PILLBUG};
set<Direction> westernDirection = {NW, SW, W};

/*
 *Use this to set a player to a cpu
 *Initialized to human players by default
 */
void Arena::setPlayer(int playerNum, Heuristic& playerHeuristic) {
	if (playerNum == 1) {
		playerOneCPU = true;
		heuristics[0] = playerHeuristic;
	} else if (playerNum == 2) {
		playerTwoCPU = true;
		heuristics[1] = playerHeuristic;
	} else {
		cout << "Illegal player set in Arena::setPlayer()" << endl;
		throw 3;
	}
}

//assumes that Arena::currentGameState has not yet made the 
//specified move
string Arena::convertToNotation(MoveInfo move){
		
	//first create the notation of the current piece
	PieceColor oldColor = currentGameState.findTopPieceColor(move.oldPieceLocation);
	PieceName oldName = move.pieceName;
	string notation = colorNotation[oldColor] + nameNotation[oldName];
	notation += findTopPieceOrder(move.oldPieceLocation);
	Bitboard test;
	
	// if landing on top of another piece, use that in the notation
	if (currentGameState.upperLevelPieces.containsAny(move.newPieceLocation)){
		test.initializeTo(move.newPieceLocation);
		notation += colorNotation[currentGameState.findTopPieceColor(test)];
		notation += nameNotation[currentGameState.findTopPieceName(test)];
		notation += findTopPieceOrder(test);
		return notation;
	}

	//find a relative direction to connect to (if applicable)
	for (auto direction: hexagonalDirections) {
		test.initializeTo(move.newPieceLocation);
		test.shiftDirection(direction);
		if (currentGameState.allPieces.containsAny(test)) 
			notation += " ";
			if (westernDirection.find(direction) != westernDirection.end())
				notation += dirNotation[direction];
			notation += colorNotation[currentGameState.findTopPieceColor(test)];
			notation += nameNotation[currentGameState.findTopPieceName(test)];
			notation += findTopPieceOrder(test);
			if (westernDirection.find(direction) == westernDirection.end())
				notation += dirNotation[direction];
			return notation;
	}
	return notation;
}

//assumes that Arena::currentGameState has not yet made the specified move
MoveInfo Arena::convertFromNotation(string notation) {
	string pieceIdentifier = "";
	string newLocation = "";
	MoveInfo move;

	string * ptr = &pieceIdentifier;
	//split into strings delimited by space
	for (unsigned i = 0 ; i < notation.size() ; i++) {
		if (notation[i] == ' '){
			ptr = &newLocation;
			continue;
		}
		*ptr += notation[i];
	}

	//interpret the first half of the string
	move.pieceName = nameNotationReverse[to_string(pieceIdentifier[1])];
	PieceColor color = colorNotationReverse[to_string(pieceIdentifier[0])];
	string pieceOrderString = "";
	if (pieceIdentifier.size() > 2) {
		pieceOrderString = pieceIdentifier[2];
	}

	//determine which piece the string corresponds to 
	Bitboard foundPiece(*currentGameState.getPieces(move.pieceName));
	foundPiece.intersectionWith(*currentGameState.getPieces(color));
	int p = 0;
	if (pieceOrderString.size()) p = stoi(pieceOrderString);
	// since many pieces of a given name and color
	// can be placed in the hive, determine which one it is
	for (auto piece: foundPiece.splitIntoBitboards()) {
		string s = pieceOrderStack[piece.hash()].top();
		if (s == pieceOrderString)  {
			foundPiece = piece;
			break;
		}
	}
	move.oldPieceLocation = foundPiece;

	//if the second half of the string exists
	if (newLocation.size() == 0 )  {
		move.newPieceLocation = startSpawnBoard;
	} else {
		set<char> symbols {'\\','/','-'};
		bool isWesternDirection = symbols.find(newLocation[0]) != symbols.end();

		//interpret the second half of the string
		string newNameString = to_string(pieceIdentifier[1 + isWesternDirection]);
		PieceName newName = nameNotationReverse[newNameString];
		string newColorString = to_string(pieceIdentifier[0 + isWesternDirection]);
		PieceColor newColor = colorNotationReverse[newColorString];

		//determine possible pieces
		foundPiece.initializeTo(*currentGameState.getPieces(newColor));
		foundPiece.intersectionWith(*currentGameState.getPieces(newName));

		//determine whether it is a directional move or a climb
		string pieceOrderString = "";
		bool containsDirection = false;
		char dir;
		for (char c: symbols) 
			if (newLocation.find(c) != string::npos)  {
				containsDirection = true;
				dir = c;
				break;
			}
			
		if (newLocation.size() > (2 + containsDirection)) {
			pieceOrderString = newLocation[2 + isWesternDirection];
		}

		// since many pieces of a given name and color
		// can be placed in the hive, determine which one it is
		for (auto piece: foundPiece.splitIntoBitboards()) {
			string s = pieceOrderStack[piece.hash()].top();
			if (s == pieceOrderString)  {
				foundPiece = piece;
				break;
			}
		}

		//follow the instruction given in notation
		if (containsDirection) {
			Direction direction = dirNotationReverse[dir][isWesternDirection];
			foundPiece.shiftDirection(direction);
		}

		move.newPieceLocation = foundPiece;

	}
	return move;
}


//Assumes that specified move is legal
void Arena::makeMove(string move){
	MoveInfo moveInfo = convertFromNotation(move);
	makeMove(moveInfo);
};

//Assumes that move is legal
void Arena::makeMove(MoveInfo move){
	Bitboard movingPiece = move.oldPieceLocation;
	Bitboard newLocation = move.newPieceLocation;
	string pieceOrderString = "";

	//if spawning a piece
	if (movingPiece.count() == 0) {	

		//find certain pieces that do not occur more than once
		if (singlePieces.find(move.pieceName) == singlePieces.end()){
			//find all identical pieces and add 1 to the count
			PieceColor color = currentGameState.findTopPieceColor(move.oldPieceLocation);
			PieceName name = move.pieceName;
			Bitboard identicalPieces = *currentGameState.getPieces(name);
			identicalPieces.intersectionWith(*currentGameState.getPieces(color));
			pieceOrderString = to_string(identicalPieces.count() + 1);
		}
		//update the pieceOrderStack
		pieceOrderStack[movingPiece.hash()].push(pieceOrderString);
	}
	
	//remove from old location, insert into new
	pieceOrderString = findTopPieceOrder(movingPiece);
	pieceOrderStack[movingPiece.hash()].pop();
	pieceOrderStack[newLocation.hash()].push(pieceOrderString);
	//update 
	moveHistory.push(move);
	currentGameState.replayMove(move);
};

//Assumes that the piece is already in the hive
string Arena::findTopPieceOrder(Bitboard piece){
	return pieceOrderStack[piece.hash()].top();
};
