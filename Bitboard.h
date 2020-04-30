#pragma once
#include <unordered_set> 
#include <random>
#include <list>
#include <vector>
#include <unordered_map>
#include <map>
#include <iostream>
#include <set>
#include "constants.h"


using namespace std;
class Bitboard {

//	map<int, unsigned long long> internalBoards;

	set<int> internalBoardCache;
	unsigned long long internalBoards[BITBOARD_SIZE];

	void unionWith(int , unsigned long long);
	void replaceWith(int, unsigned long long);
  public:
	Bitboard() {};
	Bitboard(unordered_map<int, unsigned long long>);

	void initialize(unordered_map <int, unsigned long long>);
	void initializeTo(Bitboard&);
	void setBoard(int , unsigned long long);

	void convertToHexRepresentation(Direction, int);
	void shiftOrthogonalDirection(Direction, int);
	void shiftDirection(Direction);
	void shiftDirection(Direction, int);

	unsigned long long createLowOverflowMask(Direction, int);
	unsigned long long adjustOverflowMask(Direction, int, bool, unsigned long long);


	void floodFillStep(Bitboard&, Bitboard&);
	void floodFill(Bitboard&);

	const pair <const int , const unsigned long long> getLeastSignificantBit() const;

	void pruneCache();
	void clear();

	void unionWith(Bitboard&);
	void intersectionWith(Bitboard&);
	void xorWith(Bitboard&);
	void notIntersectionWith(Bitboard&);
	bool containsAny(Bitboard&);

	bool equals(Bitboard&);

	void duplicateBoard(list<Direction>);

	int count();

	Bitboard getPerimeter();
	Bitboard slowGetPerimeter();

	unordered_map <int , vector <unsigned long long> > split();
	list <Bitboard> splitIntoBitboards();

	vector <Bitboard> splitIntoConnectedComponents();

	void print();
	int hash();
	
	int getRandomBoardIndex();
	Bitboard getRandom();

	bool operator ==(Bitboard &other) {
		return equals(other);
	}


	inline unsigned long long operator [](int index) {
		if (internalBoardCache.find(index) != internalBoardCache.end()) 
			return internalBoards[index];
		return 0;
	}

	//overload the assigment operator to O(k) instead of O(n)
	void operator = (const Bitboard& other) {
		internalBoardCache = other.internalBoardCache;
		for (int boardIndex : internalBoardCache) {
			internalBoards[boardIndex] = other.internalBoards[boardIndex];
		}
	}	

	bool operator == (const Bitboard& other) const {
		set <int> combined;
		for (auto i: internalBoardCache) {
			if (internalBoards[i] != 0) combined.insert(i);
		}
		for (auto i: other.internalBoardCache) {
			if (other.internalBoards[i] != 0) combined.insert(i);
		}
		if (other.internalBoardCache.size() != combined.size() ||
			internalBoardCache.size() != combined.size())
			return false;
		for (int i: combined) {
			if (other.internalBoards[i] != internalBoards[i])
				return false;
		}
		return true;
	}
};


