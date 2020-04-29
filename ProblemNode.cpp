#include <list>
#include <iterator>
#include <unordered_map>
#include "Bitboard.h"
#include "ProblemNode.h"

int numDirections = 6;
int lowerLeftGate = (BITBOARD_ROWS - 1)*BITBOARD_COLS;

//TODO: CHECK THESE IF ANY ERROR IN BEETLE MOVE
unordered_map <Direction, unsigned long long> gateInDirection = {
	 {Direction::SE, 524800u},
	 {Direction::SW, 132096u},
	 {Direction::W, 33554944u}, 
	 {Direction::NW, 67239936u},
	 {Direction::NE, 34078720u},
	 {Direction::E, 67109888u}
	};

//gates are structures that create problemNodes
//they are centered around 2,2 at board index lowerLeftGate
Bitboard gates[] = {
	Bitboard({{lowerLeftGate, 134479872u}}),
	Bitboard({{lowerLeftGate, 264192u}}),
	Bitboard({{lowerLeftGate, 262148u}}),
	Bitboard({{lowerLeftGate, 262400u}}),
	Bitboard({{lowerLeftGate, 17039360u}}),
	Bitboard({{lowerLeftGate, 17180131328u}})
};


//problemNodes are places in the board that do
//not have the freedom to move along all edges
//they are centered around 2,2 at board index lowerLeftGate 
Bitboard potentialProblemNodes[] = {
	Bitboard({{lowerLeftGate,67633152u}}),
	Bitboard({{lowerLeftGate,525312u}}),
	Bitboard({{lowerLeftGate,1536u}}),
	Bitboard({{lowerLeftGate,131584u}}),
	Bitboard({{lowerLeftGate,33685504u}}),
	Bitboard({{lowerLeftGate,100663296u}})
};

using namespace std;


ProblemNodeContainer::ProblemNodeContainer(Bitboard * pieces) {
	allPieces = pieces;
}

void ProblemNodeContainer::insert(Bitboard& problemNodes) {
	int problemNodeHash = problemNodes.hash();

	if (problemNodeExists(problemNodes)) return;

	problemNodeHashes.insert(problemNodeHash);

	for (auto map: problemNodes.split()){
		for (auto piece: map.second){
			int hashInt = hash(map.first, piece);

			if (problemNodes.count() != 2) {
				cout << "Attempting to insert an incorrectly sized problemNode" << endl;
				throw 19;
			}
			//assigned the hash to a map for O(1) access
			locationHashTable[hashInt].push_front(problemNodes);
		}
	}	
}


bool ProblemNodeContainer::problemNodeExists(Bitboard& problemNode) {
	return problemNodeHashes.find(problemNode.hash()) != problemNodeHashes.end();
}
void ProblemNodeContainer::clear() {
	locationHashTable.clear();
	problemNodeHashes.clear();
}

int ProblemNodeContainer::hash(int boardIndex, unsigned long long piece) {
	return boardIndex + (__builtin_clzll(piece) << 8); 
	// 0x04d7651f <- might be faster to use this ;-)
}


void ProblemNodeContainer::remove(Bitboard & problemNodes) {
	problemNodeHashes.erase(problemNodes.hash());
}
void ProblemNodeContainer::removePiece( Bitboard & piece) {
	if (piece.count() != 1) {
		cout << "Attempting to remove incorrectly sized piece" << endl;
		throw 14;
	}

	//TODO: make this unneccessary
	piece.pruneCache();

	int pieceHash = piece.hash();

	Bitboard testUpdate;
	if (locationHashTable.find(pieceHash) != locationHashTable.end()) {
		for (auto board: locationHashTable[pieceHash]){
			testUpdate.unionWith(board);
		}
	testUpdate.unionWith(piece);
	}

	int boardIndex = piece.getRandomBoardIndex();

	Bitboard pieceRemoved(*allPieces);
	pieceRemoved.notIntersectionWith(piece);
	Bitboard * temp = allPieces;

	

	list <Bitboard> problemNodesCollection = getProblemNodesAtLocation(boardIndex, 
			piece[boardIndex]);

	

	for (auto problemNodes: problemNodesCollection) {
		//remove from problemNodeHashes
		remove(problemNodes);
		//add locations to update list over every problem node location
		testUpdate.unionWith(problemNodes);
	}


	allPieces = &pieceRemoved;
	//update to see if there are more problem nodes at those locations
	updateVisible(testUpdate);

	allPieces = temp;
}

//call this only once! very slow and inefficient
//TODO: programatically enforce above rule
void ProblemNodeContainer::findAllProblemNodes() {
	Bitboard testUpdate;
	for (auto map: allPieces -> split() ) {
		for (unsigned long long board: map.second) {
			for (auto problemNodes: getProblemNodesAtLocation(map.first, board)){
				insert(problemNodes);
				testUpdate.unionWith(problemNodes);
			}	
		}
	}
	updateVisible(testUpdate);
}

//TODO: test and remove pruneCache
void ProblemNodeContainer::insertPiece(Bitboard & piece) {
	piece.pruneCache();
	int boardIndex = piece.getRandomBoardIndex();
	unsigned long long board = piece[boardIndex];

	Bitboard testUpdate;
	for (auto problemNodes : getProblemNodesAtLocation(boardIndex, board)){
		insert(problemNodes);
		testUpdate.unionWith(problemNodes);
	}
	updateVisible(testUpdate);
}

void ProblemNodeContainer::updateVisible(Bitboard& locations) {
	//finds out whether the location on the bit board should be
	//set in problemNodes
	visibleProblemNodes.unionWith(locations);
	visibleProblemNodes.xorWith(locations);

	for (auto location: locations.splitIntoBitboards()){
		int hashInt = location.hash();

		auto problemNodes = locationHashTable[hashInt].begin();

		while (problemNodes != locationHashTable[hashInt].end()) {
			Bitboard testProblemNodes(*problemNodes);
			testProblemNodes.notIntersectionWith(*allPieces);

			if (testProblemNodes.count() == 2) {

				if (problemNodeExists(testProblemNodes)) {
					visibleProblemNodes.unionWith(testProblemNodes);
					break;
				} else {
					locationHashTable[hashInt].erase(problemNodes++);
				}
			} else 
				problemNodes++;
		}

		if (locationHashTable.size() == 0) locationHashTable.erase(hashInt);
	}
}

list <Bitboard>
ProblemNodeContainer::getProblemNodesAtLocation(int boardIndex, unsigned long long
														   piece){	

												
	list <Bitboard> retList;
	int leadingZeroesCount = __builtin_clzll(piece);

	int xShift = ((63) - leadingZeroesCount )% BITBOARD_WIDTH - 2;
	int yShift = ((63) - leadingZeroesCount )/ BITBOARD_HEIGHT - 2;

	xShift += (BITBOARD_WIDTH * (boardIndex % BITBOARD_COLS));
	yShift += BITBOARD_HEIGHT * (BITBOARD_ROWS - 1 - (boardIndex / BITBOARD_ROWS));	

	Bitboard testGate;
	Bitboard testProblemNodes;
	for( int i = 0; i < numDirections; i++){

		
		//create and shift into place
		//TODO: make a 2d shift function
		testGate.initializeTo(gates[i]);
		testGate.shiftDirection(Direction::N, yShift);
		testGate.shiftDirection(Direction::E, xShift);
		testGate.convertToHexRepresentation(Direction::NE,yShift);
		
		//see if all the bits in gateBitboard are set
		testGate.intersectionWith(*allPieces);

		if (testGate.count() == 2){

			
			//create and shift into place
			testProblemNodes.initializeTo(potentialProblemNodes[i]);
			testProblemNodes.shiftDirection(Direction::N, yShift);
			testProblemNodes.shiftDirection(Direction::E, xShift);
			testProblemNodes.convertToHexRepresentation(Direction::NE,yShift);

			//Add nodes to allProblemNodes (disregarding visibility);
			if (testProblemNodes.count() == 2 ) retList.push_front(testProblemNodes);

		}
	}
	return retList;
}

Bitboard ProblemNodeContainer::getPerimeter(Bitboard& pieces) {
	Bitboard perimeter;

	//first assume every direction is in perimeter
	perimeter = pieces.getPerimeter();

	for (auto piece: pieces.splitIntoBitboards()) {
		for (auto restrictedNodes: locationHashTable[piece.hash()]) {
			//remove every restriction found
			perimeter.notIntersectionWith(restrictedNodes);
		}
	}
	return perimeter;
}

bool ProblemNodeContainer::contains(Bitboard& piece){
	return (locationHashTable.find(piece.hash()) != locationHashTable.end());
}

