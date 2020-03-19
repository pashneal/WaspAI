#include <unordered_map>
#include <vector>

using namespace std;
enum PieceName{
	GRASSHOPPER,
	QUEEN,
	LADYBUG,
	PILLBUG,
	MOSQUITO,
	BEETLE,
	ANT,
	SPIDER
};

enum Direction{
	NE = 0,
	E = 1,
	SE = 2,
	SW = 3,
	W = 4,
	NW = 5,
	N = -1,
	S = -2
};


extern int dxdy[6][2];  

extern unordered_map <string, int> pieceNum; 

extern unordered_map < string , vector<Direction> > directions;  



// directions are set out like this
//
// 
//     5    ▄▟▙▄    0 
//      ▁▄▟██████▙▄▁  
//     ██grasshoppe██ 
// 4   ██G1        ██    1
//     ██w         ██
//     ██          ██
//      ▔▀▜██████▛▀▔
//    3     ▀▜▛▀      2
