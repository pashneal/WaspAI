CXX=g++
CXXFLAGSDEBUG=-Wall -Wextra -ggdb -pthread
CXXFLAGSRELEASE=-O3 -Wall -pthread
DEBUG=-o test.out
RELEASE=-o test.out

DEPENDENCIES=Bitboard.o\
	constants.o\
	GameState.o\
	Heuristic.o\
	MonteCarloTree.o\
	MoveGenerator.o\
	PieceGraph.o\
	PieceNode.o\
	ProblemNode.o\
	Test.o\
	Arena.o

debug: $(DEPENDENCIES)
	$(CXX) $(CXXFLAGSDEBUG) $(DEPENDENCIES) $(DEBUG)

# relink and compile with optimization flags
all: 
	$(CXX) $(CXXFLAGSRELEASE) *.cpp $(RELEASE)

Bitboard.o: Bitboard.cpp Bitboard.h constants.h

constants.o: constants.h constants.cpp

GameState.o: GameState.h PieceNode.h ProblemNode.h MoveGenerator.h PieceGraph.h GameState.cpp \
	constants.h

Heuristic.o: Heuristic.h Weight.h GameState.h Heuristic.cpp constants.h

MonteCarloTree.o: MonteCarloNode.h  MonteCarloTree.h MonteCarloTree.cpp constants.h

PieceGraph.o: PieceNode.h PieceGraph.h PieceGraph.cpp constants.h

ProblemNode.o: Bitboard.h ProblemNode.h ProblemNode.cpp constants.h

MoveGenerator.o: ProblemNode.h Bitboard.h MoveGenerator.h MoveGenerator.cpp constants.h

PieceNode.o: Bitboard.h PieceNode.h PieceNode.cpp constants.h

Arena.o: Arena.h Arena.cpp constants.h Heuristic.h 

Test.o: Test.h Test.cpp
	$(CXX) $(CXXFLAGSDEBUG) -c Test.cpp

clean:
	rm -f *.o
