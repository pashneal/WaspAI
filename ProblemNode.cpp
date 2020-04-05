#include <list>
#include <iterator>
#include <unordered_map>
#include "Bitboard.h"
#include "ProblemNode.h"

int numDirections = 6;
int lowerLeftGate = (BITBOARD_CONTAINER_ROWS - 1)*BITBOARD_CONTAINER_COLS;

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
//they are centered around 2,2 at board index 12
BitboardContainer gates[] = {
	BitboardContainer({{lowerLeftGate, 134479872u}}),
	BitboardContainer({{lowerLeftGate, 264192u}}),
	BitboardContainer({{lowerLeftGate, 262148u}}),
	BitboardContainer({{lowerLeftGate, 262400u}}),
	BitboardContainer({{lowerLeftGate, 17039360u}}),
	BitboardContainer({{lowerLeftGate, 17180131328u}})
};


//problemNodes are places in the board that do
//not have the freedom to move along all edges
//they are centered around 2,2 at board index 12
BitboardContainer potentialProblemNodes[] = {
	BitboardContainer({{lowerLeftGate,67633152u}}),
	BitboardContainer({{lowerLeftGate,525312u}}),
	BitboardContainer({{lowerLeftGate,1536u}}),
	BitboardContainer({{lowerLeftGate,131584u}}),
	BitboardContainer({{lowerLeftGate,33685504u}}),
	BitboardContainer({{lowerLeftGate,100663296u}})
};

using namespace std;


ProblemNodeContainer::ProblemNodeContainer(BitboardContainer * pieces) {
	allPieces = pieces;
}

void ProblemNodeContainer::insert(BitboardContainer& problemNodes) {
	int problemNodeHash = problemNodes.hash();

	if (problemNodeExists(problemNodes)) return;

	problemNodeHashes.insert(problemNodeHash);

	for (auto map: problemNodes.split()){
		for (auto piece: map.second){
			int hashInt = hash(map.first, piece);

			if (problemNodes.count() != 2) {
				throw 19;
			}
			//assigned the hash to a map for O(1) access
			//cout << piece << " -> " <<  problemNodes.internalBoards[5] <<
			//		" " << problemNodes.internalBoards[6] << endl;
			locationHashTable[hashInt].push_front(problemNodes);
		}
	}	
}

BitboardContainer ProblemNodeContainer::getLegalClimbs(BitboardContainer& initialPiece,
				  BitboardContainer& upperLevelPieces,
				  unordered_map < int , stack < pair < PieceColor, PieceName>>> stackHashTable) {

	BitboardContainer legalClimbs;

	pair <int , unsigned long long> bit = initialPiece.getLeastSignificantBit();

	int boardIndex = bit.first;
	unsigned long long piece = bit.second; 

	int leadingZeroesCount = __builtin_clzll(piece);

	int xShift = ((63) - leadingZeroesCount )% 8 - 2;
	int yShift = ((63) - leadingZeroesCount )/ 8 - 2;

	xShift += (BITBOARD_WIDTH * (boardIndex % BITBOARD_CONTAINER_COLS));
	yShift += BITBOARD_HEIGHT * (BITBOARD_CONTAINER_ROWS - 1 - (boardIndex / BITBOARD_CONTAINER_ROWS));	

	BitboardContainer testGate;
	BitboardContainer finalPiece;
	for (auto dir: hexagonalDirections) {

		//TODO: make a 2d shift function
		testGate.initialize({{lowerLeftGate ,  gateInDirection[dir]}});
		testGate.shiftDirection(Direction::N, yShift);
		testGate.shiftDirection(Direction::E, xShift);
		testGate.convertToHexRepresentation(Direction::NE,yShift);
		
		testGate.intersectionWith(upperLevelPieces);
		if (testGate.count() == 2) {
			finalPiece.initializeTo(initialPiece);
			finalPiece.shiftDirection(dir);
			int maxPieceHeight = 0;
			int minGateHeight = 100;

			for (BitboardContainer gatePiece: testGate.splitIntoBitboardContainers() ) {
				int min = stackHashTable[gatePiece.hash()].size();
				minGateHeight = (min < minGateHeight) ? min : minGateHeight;
			}

			if (upperLevelPieces.containsAny(finalPiece)) {
				maxPieceHeight = stackHashTable[finalPiece.hash()].size();
			}
			int max = stackHashTable[initialPiece.hash()].size();

			if (upperLevelPieces.containsAny(initialPiece) && 
				 max > maxPieceHeight)  {
				maxPieceHeight = max;
			}

			if (maxPieceHeight > minGateHeight) {
				legalClimbs.unionWith(finalPiece);
			}

		}
		
	}

	return legalClimbs;
}

bool ProblemNodeContainer::problemNodeExists(BitboardContainer& problemNode) {
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


void ProblemNodeContainer::remove(BitboardContainer & problemNodes) {
	problemNodeHashes.erase(problemNodes.hash());
}
void ProblemNodeContainer::removePiece( BitboardContainer & piece) {
	if (piece.count() != 1) {
		throw 14;
	}

	//TODO: make this unneccessary
	piece.pruneCache();

	int pieceHash = piece.hash();

	BitboardContainer testUpdate;
	if (locationHashTable.find(pieceHash) != locationHashTable.end()) {
		for (auto board: locationHashTable[pieceHash]){
			testUpdate.unionWith(board);
		}
	testUpdate.unionWith(piece);
	}

	int boardIndex = *(piece.internalBoardCache.begin());

	BitboardContainer pieceRemoved(*allPieces);
	pieceRemoved.notIntersectionWith(piece);
	BitboardContainer * temp = allPieces;

	

	list <BitboardContainer> problemNodesCollection = getProblemNodesAtLocation(boardIndex, 
			piece.internalBoards[boardIndex]);

	

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
	BitboardContainer testUpdate;
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
void ProblemNodeContainer::insertPiece(BitboardContainer & piece) {
	piece.pruneCache();
	int boardIndex = *(piece.internalBoardCache.begin());
	unsigned long long board = piece.internalBoards[boardIndex];

	BitboardContainer testUpdate;
	for (auto problemNodes : getProblemNodesAtLocation(boardIndex, board)){
		insert(problemNodes);
		testUpdate.unionWith(problemNodes);
	}
	updateVisible(testUpdate);
}

void ProblemNodeContainer::updateVisible(BitboardContainer& locations) {
	//finds out whether the location on the bit board should be
	//set in problemNodes
	visibleProblemNodes.unionWith(locations);
	visibleProblemNodes.xorWith(locations);

	for (auto location: locations.splitIntoBitboardContainers()){
		int hashInt = location.hash();

		auto problemNodes = locationHashTable[hashInt].begin();

		while (problemNodes != locationHashTable[hashInt].end()) {
			BitboardContainer testProblemNodes(*problemNodes);
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

list <BitboardContainer>
ProblemNodeContainer::getProblemNodesAtLocation(int boardIndex, unsigned long long
														   piece){	

												
	list <BitboardContainer> retList;
	int leadingZeroesCount = __builtin_clzll(piece);

	int xShift = ((63) - leadingZeroesCount )% 8 - 2;
	int yShift = ((63) - leadingZeroesCount )/ 8 - 2;

	xShift += (BITBOARD_WIDTH * (boardIndex % BITBOARD_CONTAINER_COLS));
	yShift += BITBOARD_HEIGHT * (BITBOARD_CONTAINER_ROWS - 1 - (boardIndex / BITBOARD_CONTAINER_ROWS));	

	BitboardContainer testGate;
	BitboardContainer testProblemNodes;
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

BitboardContainer ProblemNodeContainer::getPerimeter(BitboardContainer& pieces) {
	BitboardContainer perimeter;

	//first assume every direction is in perimeter
	perimeter = pieces.getPerimeter();

	for (auto piece: pieces.splitIntoBitboardContainers()) {
		for (auto restrictedNodes: locationHashTable[piece.hash()]) {
			//remove every restriction found
			perimeter.notIntersectionWith(restrictedNodes);
		}
	}
	return perimeter;
}

bool ProblemNodeContainer::contains(BitboardContainer& piece){
	return (locationHashTable.find(piece.hash()) != locationHashTable.end());
}

