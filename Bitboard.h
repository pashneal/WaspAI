#pragma once
#include <list>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <set>
#include "constants.h"

using namespace std;
class BitboardContainer {

  public:
	//TODO: figure out shift direction O(n) sort so you can get rid of log(n) ordered set
	set<int> internalBoardCache;
	unsigned long long int internalBoards[16];

	//TODO: make internalBoardCache safe so we don't have to initialize
	BitboardContainer() {};
	BitboardContainer(unordered_map<int, unsigned long long>);

	void initialize(unordered_map <int, unsigned long long>);
	void initializeTo(BitboardContainer&);
	void setBoard(int , unsigned long long);

	void convertToHexRepresentation(Direction, int);
	void shiftOrthogonalDirection(Direction, int);
	void shiftDirection(Direction);
	void shiftDirection(Direction, int);

	unsigned long long createLowOverflowMask(Direction, int);
	unsigned long long adjustOverflowMask(Direction, int, bool, unsigned long long);


	void floodFillStep(BitboardContainer&, BitboardContainer&);
	void floodFill(BitboardContainer&);

	pair <int , unsigned long long> getLeastSignificantBit();

	void pruneCache();
	void clear();

	void unionWith(BitboardContainer&);
	void intersectionWith(BitboardContainer&);
	void xorWith(BitboardContainer&);
	void notIntersectionWith(BitboardContainer&);
	bool containsAny(BitboardContainer&);

	bool equals(BitboardContainer&);

	void duplicateBoard(list<Direction>);

	int count();

	BitboardContainer getPerimeter();

	unordered_map <int , vector <unsigned long long> > split();
	list <BitboardContainer> splitIntoBitboardContainers();

	vector <BitboardContainer> splitIntoConnectedComponents();

	void print();
	int hash();
	
	bool operator ==(BitboardContainer &other) {
		return equals(other);
	}

	//overload the assigment operator to O(k) instead of O(n)
	void operator = (const BitboardContainer& other) {
		internalBoardCache = other.internalBoardCache;
		for (int boardIndex : internalBoardCache) {
			internalBoards[boardIndex] = other.internalBoards[boardIndex];
		}
	}
};




