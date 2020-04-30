#include "Arena.h"

unordered_map <char, PieceColor> colorNotationReverse {
	{'w', PieceColor::WHITE},
	{'b', PieceColor::BLACK},
};
unordered_map <PieceColor, string> colorNotation {
	{PieceColor::WHITE, "w"},
	{PieceColor::BLACK, "b"},
};
unordered_map <char, vector<Direction>> dirNotationReverse{
	{'\\', {Direction::SE,Direction::NW}},
	{'-',  {Direction::E,Direction::W}},
	{'/',  {Direction::NE, Direction::SW }}
};
unordered_map <Direction, string> dirNotation {
	{Direction::NW ,"\\"},
	{Direction::NE ,"/"},
	{Direction::SW ,"/"},
	{Direction::SE ,"\\"},
	{Direction::W  ,"-"},
	{Direction::E  ,"-"},
};

unordered_map<char, PieceName> nameNotationReverse{ 
	{'G', PieceName::GRASSHOPPER},
	{'Q',		 PieceName::QUEEN},
	{'L', 	 PieceName::LADYBUG},
	{'P', 	 PieceName::PILLBUG},	
	{'M',	 PieceName::MOSQUITO},
	{'B',  	 PieceName::BEETLE},
	{'A',     	 PieceName::ANT},
	{'S',  	 PieceName::SPIDER},
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

//assumes that Arena::currentGameState has not yet made the specified move
string Arena::convertToNotation(MoveInfo move){
	//first create the notation of the current piece
	PieceColor oldColor;

	PieceName oldName = move.pieceName;

	if (move == MoveInfo()) return "pass";
	//if not spawning
	if (move.oldPieceLocation.count() == 1)
		oldColor = currentGameState.findTopPieceColor(move.oldPieceLocation);
	else 
		oldColor = currentGameState.turnColor;

	
	string notation = colorNotation[oldColor] + nameNotation[oldName];
	//if not spawning
	if (move.oldPieceLocation.count() == 1){
		notation += findTopPieceOrder(move.oldPieceLocation);
	} else {
		if (singlePieces.find(oldName) == singlePieces.end())
			notation += to_string(countPieces(oldColor, oldName) + 1);
	}

	Bitboard test;
	
	
	// if landing on top of another piece, use that in the notation
	if (currentGameState.allPieces.containsAny(move.newPieceLocation)){
		test.initializeTo(move.newPieceLocation);
		notation += " ";
		notation += colorNotation[currentGameState.findTopPieceColor(test)];
		notation += nameNotation[currentGameState.findTopPieceName(test)];
		notation += findTopPieceOrder(test);
		return notation;
	}

	//find a relative direction to connect to (if applicable)
	for (auto direction: hexagonalDirections) {
		test.initializeTo(move.newPieceLocation);
		test.shiftDirection(direction);
		if (currentGameState.allPieces.containsAny(test)) {
			notation += " ";
			Direction opposite = oppositeDirection[direction];
			//use the direction that is opposite in notation
			if (westernDirection.find(opposite) != westernDirection.end())
				notation += dirNotation[opposite];
			notation += colorNotation[currentGameState.findTopPieceColor(test)];
			notation += nameNotation[currentGameState.findTopPieceName(test)];
			notation += findTopPieceOrder(test);
			if (westernDirection.find(opposite) == westernDirection.end())
				notation += dirNotation[opposite];
			return notation;
		}
	}
	
	return notation;
}

//assumes that Arena::currentGameState has not yet made the specified move
MoveInfo Arena::convertFromNotation(string notation) {
	string pieceIdentifier = "";
	string newLocation = "";
	MoveInfo move;

	if (notation == "pass") return move;

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
	move.pieceName = nameNotationReverse[pieceIdentifier[1]];
	PieceColor color = colorNotationReverse[pieceIdentifier[0]];

	string pieceOrderString = "";
	if (pieceIdentifier.size() > 2) {
		pieceOrderString = pieceIdentifier[2];
	}

	//determine which piece the string corresponds to 
	Bitboard foundPieces(*currentGameState.getPieces(move.pieceName));
	foundPieces.intersectionWith(*currentGameState.getPieces(color));
	// since many pieces of a given name and color
	// can be placed in the hive, determine which one it is
	Bitboard foundPiece;
	for (auto piece: foundPieces.splitIntoBitboards()) {
		auto description = pieceOrders[piece.hash()].back(); 
		if (std::get<0>(description) == move.pieceName &&
			std::get<1>(description) == color &&
			std::get<2>(description) == pieceOrderString)  {
			foundPiece = piece;
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
		char newNameString = newLocation[1 + isWesternDirection];
		PieceName newName = nameNotationReverse[newNameString];
		char newColorString = newLocation[0 + isWesternDirection];
		PieceColor newColor = colorNotationReverse[newColorString];

		//determine possible pieces
		foundPieces.initializeTo(*currentGameState.getPieces(newColor));
		foundPieces.intersectionWith(*currentGameState.getPieces(newName));
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
		foundPiece.clear();
		for (auto piece: foundPieces.splitIntoBitboards()) {
			auto description = pieceOrders[piece.hash()].back(); 
			if (std::get<0>(description) == newName &&
				std::get<1>(description) == newColor &&
				std::get<2>(description) == pieceOrderString)  {
				foundPiece = piece;
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
	moveHistoryNotation.push_back(move);
	MoveInfo moveInfo = convertFromNotation(move);
	makeMove(moveInfo);
};

//Assumes that move is legal
void Arena::makeMove(MoveInfo move){
	moveHistoryNotation.push_back(convertToNotation(move));

	Bitboard movingPiece = move.oldPieceLocation;
	Bitboard newLocation = move.newPieceLocation;
	string pieceOrderString = "";
	
	PieceColor color;
	PieceName name = move.pieceName;
	//if spawning a piece
	if (movingPiece.count() == 0) {	
		color = currentGameState.turnColor;

		//find certain pieces that do not occur more than once
		if (singlePieces.find(move.pieceName) == singlePieces.end()){
			//find all identical pieces and add 1 to the count
			int num = countPieces(color, name);
			pieceOrderString = to_string(num + 1);

		}
	} else {
		color = currentGameState.findTopPieceColor(move.oldPieceLocation);
		pieceOrderString = findTopPieceOrder(movingPiece);
	}
	
	//remove from old location, insert into new
	if (movingPiece.count())
		pieceOrders[movingPiece.hash()].pop_back();

	pieceOrders[newLocation.hash()].push_back(make_tuple(name, color, pieceOrderString));
	//update 
	moveHistory.push_back(move);
	currentGameState.replayMove(move);
};

//Assumes that the piece is already in the hive
string Arena::findTopPieceOrder(Bitboard piece){
	return std::get<2>(pieceOrders[piece.hash()].back());
};

int Arena::countPieces(PieceColor color, PieceName name){

	Bitboard pieces = *currentGameState.getPieces(color);
	pieces.intersectionWith(*currentGameState.getPieces(name));
	int amount = 0;
	if (pieces.count()){
		for (Bitboard piece : pieces.splitIntoBitboards()) {
			for (auto description: pieceOrders[piece.hash()]){
				if (std::get<0>(description)== name &&  std::get<1>(description) == color) {
					amount++;
				}
			}
		}
	}
	return amount;
}
