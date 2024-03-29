#include "Arena.h"
#include <algorithm>

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
void Arena::setPlayer(int playerNum, MonteCarloTree& searchAlgo) {
	if (playerNum == 0) {
		playerOneCPU = true;
		CPU[0].initializeTo(searchAlgo);
	} else if (playerNum == 1) {
		playerTwoCPU = true;
		CPU[1].initializeTo(searchAlgo);
	} else {
		cout << "Illegal player set in Arena::setPlayer()" << endl;
		throw 3;
	}
}

//assumes that Arena::currentGameState has not yet made the specified move
string Arena::convertToNotation(MoveInfo move){

	if (move == MoveInfo()) return "pass";
	//if not spawning
	PieceColor oldColor;
	PieceName oldName = move.pieceName;
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


	Bitboard piecesExceptCurrent(currentGameState.allPieces);
	piecesExceptCurrent.notIntersectionWith(move.oldPieceLocation);

	//find a relative direction to connect to (if not the first piece)
	for (auto direction: hexagonalDirections) {
		test.initializeTo(move.newPieceLocation);
		test.shiftDirection(direction);

		if (piecesExceptCurrent.containsAny(test)) {
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
		if (pieceOrderString == "1" && singlePieces.find(move.pieceName) != singlePieces.end())
			pieceOrderString = "";
	}

	//determine which piece the string corresponds to 
	Bitboard foundPieces(*currentGameState.getPieces(move.pieceName));
	foundPieces.intersectionWith(*currentGameState.getPieces(color));

	// since many pieces of a given name and color
	// can be placed in the hive, determine which one it is
	Bitboard foundPiece;
	for (auto& piece: foundPieces.splitIntoBitboards()) {
	
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

		for (char c: symbols) {
			if (newLocation.find(c) != string::npos)  {
				containsDirection = true;
				dir = c;
				break;
			}
		}

		if ((int)newLocation.size() > (2 + containsDirection)) {
			pieceOrderString = newLocation[2 + isWesternDirection];
		}
		// since many pieces of a given name and color
		// can be placed in the hive, determine which one it is
		foundPiece.clear();
		for (auto& piece: foundPieces.splitIntoBitboards()) {
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
	if (move.newPieceLocation == Bitboard() && move.oldPieceLocation == Bitboard()) {
		cout << "Parse error for input notation" << endl; 
		cout <<  notation << endl;
 		throw  55;
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
	moveHistoryNotation.push_back(convertToNotation(move));

	//if no move made
	if (move == MoveInfo()) {
		moveHistory.push_back(move);
		currentGameState.replayMove(move);
		cout << "HERREE" << endl;
		return;
	}

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
		color = currentGameState.findTopPieceColor(movingPiece);
		pieceOrderString = findTopPieceOrder(movingPiece);
	}
	
	//remove from old location, insert into new
	if (movingPiece.count())
		pieceOrders.at(movingPiece.hash()).pop_back();

	if (newLocation.count() == 0){
		cout << move.toString("") << endl;
		throw 3;
	}
	pieceOrders[newLocation.hash()].push_back(make_tuple(name, color, pieceOrderString));

	//update 
	moveHistory.push_back(move);
	currentGameState.replayMove(move);
};

void Arena::undo(string move){
	MoveInfo moveInfo = convertFromNotation(move);
	undo(moveInfo);
}

void Arena::undo(MoveInfo move){
	moveHistory.pop_back();
	moveHistoryNotation.pop_back();
	auto pieceOrder = pieceOrders.at(move.newPieceLocation.hash()).back();
	if (!(move.oldPieceLocation == Bitboard()))
		pieceOrders[move.oldPieceLocation.hash()].push_back(pieceOrder);
	pieceOrders.at(move.newPieceLocation.hash()).pop_back();
	currentGameState.undoMove(move);
}

//Assumes that the piece is already in the hive
string Arena::findTopPieceOrder(Bitboard piece){
	return std::get<2>(pieceOrders.at(piece.hash()).back());
};

int Arena::countPieces(PieceColor color, PieceName name){
	Bitboard pieces = *currentGameState.getPieces(color);
	pieces.intersectionWith(*currentGameState.getPieces(name));
	int amount = 0;
	for (Bitboard& piece : pieces.splitIntoBitboards()) {
		for (auto description: pieceOrders[piece.hash()]){
			if (std::get<0>(description)== name &&  std::get<1>(description) == color) {
				amount++;
			}
		}
	}
	return amount;
}

/*
 * Allows both agents to take a turn. If it is a human agent,
 * require correct input from the console. If it is a computer agent
 * require correct input from the search algorithm
 *
 * returns true if the game is over
 */
bool Arena::battle(bool silent) {
	bool vals[] = {playerOneCPU, playerTwoCPU};

	for (int i = 0 ; i < 2 ; i++) {
		if ((PieceColor)i != currentGameState.turnColor) continue;
		bool isCPU = vals[i];
		MoveInfo selectedMove;
		vector<MoveInfo> moves = currentGameState.generateAllMoves();

		if (!moves.size()) {
			currentGameState.replayMove(selectedMove);
			if (!silent) {
				string a = (isCPU) ? "Computer" : "Human";
				cout << "[" << a << " Player " << i << "] pass" << endl;
			}
			continue;
		};

		set<string> moveStrings;
		for (auto move: moves) {
			if (!silent)
				cout << convertToNotation(move) <<endl;
			moveStrings.insert(convertToNotation(move));	
		}

		if (isCPU) {
			selectedMove = CPU[i].multiSearch(currentGameState, numCores);
			if (std::find(moves.begin(), moves.end(), selectedMove) == moves.end())  {
				cout << "SearchAlgorithm " << i << " returned an illegal move " << endl;
				throw "hello darkness my old friend";
			}
			
		} else {

			string inputMove;
			do {
				cout << "[Human Player " << i << "] please enter a valid move: "<< endl;
				getline(cin, inputMove);
				cout << endl;
			} while (moveStrings.find(inputMove) == moveStrings.end() );
			selectedMove = convertFromNotation(inputMove);

		}

		if (!silent) {
			string a = (isCPU) ? "Computer" : "Human";
			cout << "[" << a << " Player " << i << "] " << convertToNotation(selectedMove) << endl;
		}

		makeMove(selectedMove);

		if (currentGameState.checkDraw() || currentGameState.checkVictory() != PieceColor::NONE) {
			if (!silent) {
				cout << "Game Over" << endl;
				if (currentGameState.checkDraw())
					cout << "Draw" << endl;
				else {
					cout << "Player " << currentGameState.checkVictory() << " has won";
				}
			}
			return true;
		}
	}
	return false;
}
