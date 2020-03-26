#include <list>
#include <unordered_map>
#include "Bitboard.h"
#include "ProblemNode.h"

using namespace std;

void ProblemNodeContainer::insert(BitboardContainer problemNodes, BitboardContainer gate) {
	for (auto map: problemNodes.split()){

		int hashInt = hash(map.second[0], map.second[1]);
		

		BitboardContainer hashedBitboard;
		problemNodeMap[hashInt].push_front(gate);
		problemNodeMap[hashInt].begin() -> initializeTo(problemNodes);

		for (auto piece: map.second){
			problemNodeHashTable[hash(piece, map.first)].push_front(
					&(problemNodeMap[hashInt].front()) );
		}

	}
	
	for (auto map: gate.split()){
		//Todo here		
	}

}

void ProblemNodeContainer::clear() {
	problemNodeMap.clear();
	problemNodeHashTable.clear();
}

int hash(unsigned long long piece, int boardIndex) {
	return boardIndex + (__builtin_clzll(piece) << 8); 
	// 0x04d7651f <- might be faster to use this ;-)
}

int hash(unsigned long long piece1, unsigned long long piece2){
	return (__builtin_clzll(piece1) << 16) + (__builtin_clzll(piece2) << 8);
	//imperfect hash
}

