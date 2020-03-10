#include <unordered_map>
#include "Piece.h"
using namespace std;

unordered_map<string , string> pieceName = {{"Q","queen"},
									   {"B1","beetle"},
									   {"B2","beetle"},
									   {"G1","grasshopper"},
									   {"G2","grasshopper"},
									   {"G3","grasshopper"},
									   {"A1","ant"},
									   {"A2","ant"},
									   {"A3","ant"},
									   {"S1","spider"},
									   {"S2","spider"},
									   {"M","mosquito"},
									   {"L","ladybug"},
									   {"P","pillbug"}
};

Piece::Piece(char pieceColor, string shorthand, int xPos, int yPos) {
	color = pieceColor;
	shorthandName = shorthand;
	name = pieceName[shorthand];
	x = xPos;
	y = yPos;
};
