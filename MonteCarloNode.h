#pragma once
#include <memory>
#include "GameState.h"
#include  "Heuristic.h"

#define nodePtr shared_ptr<MonteCarloNode>
//step size for error correction
double LEARNING_RATE = 1.0;

class MonteCarloNode{
		vector <double> heuristicEvals;
	public:
		int numVisited = 0;
		double heuristicScore;
		double playoutScore;
		double maxChildScore;
		double minChildScore;
		double maxAvgScore = -1;
		bool proven = false;
		nodePtr parent;
		unordered_map <MoveInfo, nodePtr> children;
	
		
		void createChild(MoveInfo m) {
			MonteCarloNode child;
			child.parent = nodePtr(this);
			children[m] = nodePtr(&child);
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

		void clearParent() {
			parent.reset();
			for (auto child: parent->children) 
				if( child.second != nodePtr(this)) 
					child.second -> clearChildren();
		}

		void clearChildren() {
			parent.reset();
			for (auto child: children) {
				child.second -> clearChildren();
				child.second.reset();
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
			if (minExpectedScore == maxExpectedScore)
				expectedResults = 1;
			else 
				expectedResults = (heuristicScore - minExpectedScore) / 
								  (maxExpectedScore - minExpectedScore);

			// score produced by playouts normalized by best sibling playout score
			// this way expectedResults approximate best move within given options
			double actualResults = (numVisited) ? ((playoutScore)/numVisited)/maxAvgScore : 0;
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
