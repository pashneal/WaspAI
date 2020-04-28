#include "Arena.h"

unordered_map <string, PieceColor> colorNotationReverse {
	{"w", PieceColor::WHITE},
	{"b", PieceColor::BLACK},
};
unordered_map <PieceColor, string> colorNotation {
	{PieceColor::WHITE, "w"},
	{PieceColor::BLACK, "b"},
};
unordered_map <string, Direction> dirNotationReverse {
	{"\\",Direction::NW },
	{"/",Direction::NE },
	{"/",Direction::SW },
	{"\\",Direction::SE },
	{"-",Direction::W  },
	{"-",Direction::E  },
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

vector<PieceName> singlePieces = {MOSQUITO, QUEEN, LADYBUG, PILLBUG};
set<Direction> westernDirection = {NW, SW, W};
/*
 *Use this to set player to a cpu
 *Initialized to human player by default
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

//assumes that Arena::currentGameState has already made the 
//specified move
string Arena::convertToNotation(MoveInfo move){
		
	//first create the notation of the current piece
	PieceColor newColor = currentGameState.findTopPieceColor(move.newPieceLocation);
	PieceName newName = currentGameState.findTopPieceName(move.newPieceLocation);
	string notation = colorNotation[newColor] + nameNotation[newName];
	notation += findTopPieceOrder(move.newPieceLocation);
	BitboardContainer test;

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

	for (unsigned i = 0 ; i < notation.size() ; i++) {
		if (notation[i] == ' ')
			asdf;
	}
	if (newLocation.size() == 0 ) move.newPieceLocation = startSpawnBoard;
	else {
	}
}
string Arena::findTopPieceOrder(BitboardContainer piece) {
	if( pieceOrderStack.find(piece) != pieceOrderStack.end() ) {
		return to_string(pieceOrderStack[piece].top());
	}
	int i = 1;
	for (BitboardContainer pieceLocations: pieceOrder) {
		if (pieceLocations.containsAny(piece))
			return to_string(i);
		i++;
	}
	return "";
}
