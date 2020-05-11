#include "Heuristic.h"
Heuristic::Heuristic (Complexity c, set<char> expansionPieces) {
	set<char> hiveCarbon {'P','L','M'};
	if (expansionPieces != hiveCarbon) {
		cout << "NOT IMPLEMENTED YET" << endl;
		throw 10;
	}

	if (c == Complexity::SIMPLE) {
		weights.push_back(KillShotCountWeight(1));
	}else if (c != Complexity::RANDOM)  {
		cout << "NOT IMPLEMENTED YET" << endl;
		throw 10;
	}
}
void Heuristic::loadWeights(string){
}
//load from default file
void Heuristic::load(){
}
void Heuristic::saveWeights(string){
}
//save to the default file 
void Heuristic::save(){
}
void Heuristic::setGameState(GameState& parentGameState) {
	for (Weight w: weights) {
		w.initialize(parentGameState);
	}
}
//assumes game state is already initialized
vector<double> Heuristic::evaluate(MoveInfo m){
	vector <double> evals;
	for (Weight w: weights) {
		evals.push_back(w.evaluate(m));
	}
	return evals;
}

void Heuristic::train(vector<double> corrections){
	//apply corrections over weights
	//TODO: is this the correct way to do it?
	for (unsigned i = 0 ; i < corrections.size() ; i++){
		weights[i].multiplier += corrections[i];
	}
	save();
}



