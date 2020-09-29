#include "Heuristic.h"
Heuristic::Heuristic (Complexity c, set<char> pieces) {
	expansionPieces = pieces;
	complexity = c;
	
	set<char> hiveCarbon {'P','L','M'};
	if (expansionPieces != hiveCarbon) {
		cout << "NOT IMPLEMENTED YET" << endl;
		throw 10;
	}

	if (c == Complexity::SIMPLE) {
		weights.push_back(new KillShotCountWeight(1.3));
		weights.push_back(new PinnedWeight(1));
		weights.push_back(new PieceCountWeight(1));
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
void Heuristic::setGameState(GameState parentGameState) {
	if (gameState != nullptr)
		delete gameState;
	gameState = new GameState(parentGameState);
	
	for (auto w: weights) {
		w->initialize(gameState);
	}
}
//assumes game state is already initialized
vector<double> Heuristic::evaluate(MoveInfo m){
	vector <double> evals;

	gameState->replayMove(m);
	for (auto w: weights) {
		evals.push_back(w->evaluate(m));
	}
	gameState->undoMove(m);

	return evals;
}

void Heuristic::train(vector<double> corrections){
	//apply corrections over weights
	//TODO: is this the correct way to do it?
	for (unsigned i = 0 ; i < corrections.size() ; i++){
		weights[i]->correct( corrections[i]);
	}
	save();
}



