#pragma once
#include <unordered_set> 
#include <random>
#include <list>
#include <vector>
#include <unordered_map>
#include <map>
#include <iostream>
#include "constants.h"
#include <boost/container/flat_set.hpp>

using namespace std;
class Bitboard {

//	map<int, unsigned long long> internalBoards;

	boost::container::flat_set<int> internalBoardCache;
	unsigned long long internalBoards[BITBOARD_SIZE];

	//low level bit twiddling functions
	void unionWith(int , unsigned long long);
	unsigned long long createLowOverflowMask(Direction, int);
	unsigned long long adjustOverflowMask(Direction, int, bool, unsigned long long);
  public:

	Bitboard() {};
	Bitboard(unordered_map<int, unsigned long long>);


	//constructing a new Bitboard methods
	void initialize(unordered_map <int, unsigned long long>);
	void initializeTo(Bitboard&);
	void setBoard(int , unsigned long long);

	//low level bit twiddling functions for movement
	void shiftOrthogonalDirection(Direction, int);
	void shiftDirection(Direction);
	void shiftDirection(Direction, int);
	void convertToHexRepresentation(Direction, int);

	//bit twiddling function for performing breath first search
	void floodFillStep(Bitboard&, Bitboard&);
	void floodFill(Bitboard&);


	//functions for reducing Bitboard size
	void pruneCache();
	void clear();

	//operations with another Bitboard
	void unionWith(Bitboard&);
	void intersectionWith(Bitboard&);
	void xorWith(Bitboard&);
	void notIntersectionWith(Bitboard&);
	bool containsAny(Bitboard&);
	bool equals(Bitboard&);


	//finding hexagonal neighbors
	Bitboard getPerimeter();
	Bitboard slowGetPerimeter();

	//finding individual bits
	unordered_map <int , vector <unsigned long long> > split();
	list <Bitboard> splitIntoBitboards();
	list <Bitboard>::const_iterator begin();
	
	//random functions
	int getRandomBoardIndex();
	Bitboard getRandom();

	//miscellaneous useful functions
	int count() const;
	void duplicateBoard(list<Direction>);
	const pair <const int , const unsigned long long> getLeastSignificantBit() const;
	vector <Bitboard> splitIntoConnectedComponents();
	int hash();
	void print();
	
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
		boost::container::flat_set <int> combined;
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


