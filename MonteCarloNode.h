#pragma once
#include <memory>
#include "GameState.h"
#include  "Heuristic.h"

//step size for error correction
double LEARNING_RATE = 1.0;

class MonteCarloNode{
		vector <double> heuristicEvals;
	public:
		int numVisited = 0;
		double initialWeightScores;
		double playoutScores;

		shared_ptr<MonteCarloNode> parent;
		unordered_map <MoveInfo , shared_ptr<MonteCarloNode>> children;
	
		void createChild(MoveInfo m) {
			MonteCarloNode child;
			child.parent = shared_ptr<MonteCarloNode>(this);
			auto childPtr = shared_ptr<MonteCarloNode>(&child);
			children[m] = childPtr;
		}

		//Heuristic must already have gameState updated to this node
		inline void evaluate(Heuristic& h){
			h.evaluate();
			heuristicEvals = h.evaluations;
		}

		//Heuristic must already have gameState updated to this node
		void evaluateAllChildren(Heuristic& h) {
			for (auto child: children) {
				h.replayMove(child.first);
				child.second->evaluate(h);
				h.undoMove();
			}
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
				expectedResults = (initialWeightScores - minExpectedScore) / 
								  (maxExpectedScore - minExpectedScore);

			// score produced by playouts normalized by best sibling playout score
			// this way expectedResults approximate best move within given options
			double actualResults = (numVisited) ? ((playoutScores)/numVisited)/maxAvgScore : 0;
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
			retString += prefix + "initialWeightScores: ";
			retString += to_string(initialWeightScores);
			retString += "\n" + prefix + "playoutScores: ";
			retString += to_string(playoutScores);
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

		double playout(Heuristic& h);

};
