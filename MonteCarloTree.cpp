#include "MonteCarloTree.h"
void MonteCarloTree::select(){
}

void MonteCarloTree::expand() {
}

void MonteCarloTree::simulate() {
}

void MonteCarloTree::backPropagate() {
}

void MonteCarloTree::search(){
		for (int i = MonteCarloSimulations; i--; ) {
			currentParentNode = root;
			do {
				select();
			}while (currentParentNode ->children.size());
			expand();
			simulate();
			backPropagate();
		}
}

double MonteCarloTree::selectionFunction(MoveInfo m) {
	auto child = currentParentNode -> children[m];
	double estimation = (child->initialWeightScores - minWeightScores) /
			(maxWeightScores - minWeightScores);
	return (estimation*(heuristicFactor));
}
