#include <set>
#include <sstream>
#include <vector>
#include <iostream>
#include <string>
#include <unordered_map>
#include "constants.h"
#include "Piece.h"
#include "Hive.h"
#include "Bitboard.h"

using namespace std;

int main() {
	//TODO: be able to accept commands for moving and placing pieces
	//bitboard implementation of such commands
	//determine legal moves from any given position 
	//print out the board using a nice looking hex format
	string rawInput;
	bool whitePlayerTurn = true;
	bool parseCommandSuccess;
	Hive testHive;

	while (1) { 
		vector<string> command;
		printf("Type a command to play hive! \n");
		getline(cin, rawInput);
		string word;

		for (auto letter: rawInput){
			if (letter == ' '){
				command.push_back(word);
				word = "";
			}
			else {
				word = word + letter;
			}
		}
		
		if (!(word == "")) command.push_back(word);

		if (command.size() == 0) continue;

		if (whitePlayerTurn && command[0][0] != 'w'){
			cout << "\n The command did not start with w and it\'s white turn" << endl;
			continue;
		}
		
		else if (!whitePlayerTurn && command[0][0] != 'b'){
			cout << "\n The command did not start with b and it\'s white turn" << endl;
			continue;
		}
		
		for (auto i = command.begin(); i != command.end() ; ++i){
			cout << *i << " ";
		}

		parseCommandSuccess = testHive.parseCommand(command);

		if (!parseCommandSuccess){
			cout << "I did not understand the command: " ;

			cout << "please try again!" << endl;
			whitePlayerTurn = !whitePlayerTurn;
		}

		whitePlayerTurn = !whitePlayerTurn;
		testHive.printDebug();
	}
}


