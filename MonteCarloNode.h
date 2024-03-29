#pragma once
#include <memory>
#include "constants.h"
#include "GameState.h"
#include  "Heuristic.h"

#define nodePtr MonteCarloNode*
//step size for error correction

class MonteCarloNode{
		vector <double> heuristicEvals;
	public:
		int numVisited = 0;
		double playoutScore = 0;
		double maxAvgScore = -1;
		double heuristicScore;
		double maxChildScore;
		double minChildScore;
		bool proven = false;
		nodePtr parent = nullptr;
		unordered_map <MoveInfo, nodePtr> children;
	
		MonteCarloNode(){};
		
		void createChild(MoveInfo m) {
			MonteCarloNode * child = new MonteCarloNode;
			child->parent = this;
			children.insert(std::make_pair(m, child));
		}

		//Heuristic must already have gameState updated to parent of this node
		inline void evaluate(Heuristic& h, MoveInfo m){
			heuristicEvals = h.evaluate(m);
			heuristicScore = 0;
			for (double score: heuristicEvals) {
				heuristicScore += score;
			}
		}

		void evaluateAllChildren(Heuristic& h,GameState& gameState) {
			maxChildScore = -1;
			minChildScore = HUGE_VAL;
			h.setGameState(gameState);
			for (auto child: children) {
				child.second->evaluate(h,child.first);
				maxChildScore = std::max(maxChildScore, child.second->heuristicScore);
				minChildScore = std::min(minChildScore, child.second->heuristicScore);
			}
		}

		//assumes parent is not null
		void clearParent() {
			for (auto child: parent->children) 
				if( child.second != this) 
					child.second -> clearChildren();
			delete parent;
		}

		void clearChildren() {
			for (auto& child: children) {
				child.second -> clearChildren();
				delete child.second;
			}
			children.clear();
		}

		// maxExpectedScore = highest raw score of sibling nodes
		// minExpectedScore = lowest raw score of sibling nodes
		// maxAvgScore = max {sumOfScore/numVisited} for all sibling nodes
		// returns a vector of double representing Weight adjustments
		vector <double> train(double maxExpectedScore , double minExpectedScore,
								double maxAvgScore) {

			//proportion of times this node or its siblings have been expanded
			double  confidence = ((double)parent->numVisited)/ MonteCarloSimulations;
			double expectedResults;
			// score produced by node weights normalized by best sibling weights
			if (minExpectedScore >= maxExpectedScore)
				expectedResults = 1;
			else 
				expectedResults = (heuristicScore - minExpectedScore) / 
								  (maxExpectedScore - minExpectedScore);

			// score produced by playouts normalized by best sibling playout score
			// this way expectedResults approximate best move within given options
			double actualResults = (numVisited && maxAvgScore > 0) ? 
					((playoutScore)/numVisited)/maxAvgScore : 0;
			vector <double> corrections;

			for (double score: heuristicEvals) {
				double error =  (actualResults - expectedResults) ;
				corrections.push_back(score * error * confidence * LEARNING_RATE);
			}
			return corrections;
			//when convergence occurs, sum{corrections} for every node should be zero
		}
		
		string toString(int depth, string prefix) {
			string retString;
			if (prefix == "") retString += "==============ROOT==============\n";
			for (auto i : heuristicEvals) {
				retString += to_string(i) + " ";	
			}
			retString += prefix + "heurisiticScore: ";
			retString += to_string(heuristicScore);
			retString += "\n" + prefix + "playoutScores: ";
			retString += to_string(playoutScore);
			retString += "\n" + prefix + "numVisited: ";
			retString += to_string(numVisited);
			retString += "\n";
			if (depth == 0) return retString;
			prefix += "\t";
			for (auto child: children) {
				retString += prefix + "=====[[MOVE INFO]]=====";
				retString += child.first.toString(prefix);
			}
			return retString;
		}
};
