#include "Heuristic.h"
Heuristic::Heuristic (Complexity c, set<char> expansionPieces) {
	set<char> hiveCarbon {'P','L','M'};
	if (expansionPieces != hiveCarbon) {
		cout << "NOT IMPLEMENTED YET" << endl;
		throw 10;
	}

	if (c != Complexity::RANDOM)  {
		cout << "NOT IMPLEMENTED YET" << endl;
		throw 10;
	}
}
void Heuristic::loadWeights(string){
}
void Heuristic::saveWeights(string){
}

void Heuristic::setGameState(GameState& gs) {
	for (Weight w: weights) {
		w.initialize(parentGameState);
	}
}
vector<double> Heuristic::evaluate(MoveInfo m){
	vector <double> evals;
	for (Weight w: weights) {
		w.evaluate(m);
	}
	return evals;
}



