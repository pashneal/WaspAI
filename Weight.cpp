#include "Weight.h"

int Weight::recalculateMoves(Bitboard piece){
	int result = 0;
	bool pinned = parentGameState->pinned.containsAny(piece);
	bool stacked = parentGameState->upperLevelPieces.containsAny(piece);
	PieceName name = parentGameState->findTopPieceName(piece);
	bool canMove = (!pinned && !stacked) || (stacked && name == BEETLE);
	if (canMove) {
		moveGen.setGeneratingPieceBoard(&piece);
		result = moveGen.getMoves().count();
	}
	if (name == PILLBUG && !stacked) {
		auto swappableEmpty = parentGameState->getSwapSpaces(piece);
		result += swappableEmpty.first.count() * swappableEmpty.second.count();
	}

	return result;
}

void KillShotCountWeight::initialize(GameState* g) {
	Weight::initialize(g);
	watchPoints.clear();
	pinnedWatchPoints.clear();
	unpinnedWatchPoints.clear();

	//set watchpoints to queen and her surroudings
	//if there is no movement in these zones, the 
	//default value do not have to be recalculated
	queenCount = 0;
	for (auto& queen : parentGameState->queens.splitIntoBitboards()){
		//watch points for piece movements
		watchPoints.unionWith(queen);
		Bitboard queenPerimeter = queen.getPerimeter();
		watchPoints.unionWith(queenPerimeter);
		queenCount++;
		queenPerimeter.intersectionWith(parentGameState->allPieces);

		PieceColor queenColor = parentGameState->findBottomPieceColor(queen);
		//watch points for friendly piece mobility
		for (auto& piece: queenPerimeter.splitIntoBitboards()){

			if (parentGameState->findTopPieceColor(piece) == queenColor &&
			!parentGameState->upperLevelPieces.containsAny(piece)){

				if (parentGameState->pinned.containsAny(piece) ) {
					pinnedWatchPoints.unionWith(piece);
				} else {
					unpinnedWatchPoints.unionWith(piece);
				}
			}
		}
	}
	auto result = recalculate();
	scores[PieceColor::WHITE] = result[WHITE];
	scores[PieceColor::BLACK] = result[BLACK];
}

vector<int> KillShotCountWeight::recalculate() {
	vector<int> killShotCount{0,0};
	for (auto& queen : parentGameState->queens.splitIntoBitboards()){
	
		Bitboard queenPerimeter = queen.getPerimeter();
		PieceColor queenColor = parentGameState->findBottomPieceColor(queen);

		MoveGenerator moveGen(&parentGameState->allPieces);
		moveGen.setPieceStacks(&parentGameState->pieceStacks);
		moveGen.setUpperLevelPieces(&parentGameState->upperLevelPieces);

		//calculate and store default values
		queenPerimeter.intersectionWith(parentGameState->allPieces);
		for (auto& piece: queenPerimeter.splitIntoBitboards()){

			//if the pieceStack > 1, it takes too long to remove the piece
			if (parentGameState->upperLevelPieces.containsAny(piece)){
				killShotCount[queenColor]++;
				continue;
			}
			if (parentGameState->findTopPieceColor(piece) == queenColor){
				if (!parentGameState->pinned.containsAny(piece)){
					//if the unpinned friendly piece can move
					PieceName name = parentGameState->findTopPieceName(piece);
					moveGen.setGeneratingName(&name);
					moveGen.setGeneratingPieceBoard(&piece);
					if (moveGen.getMoves().count()){
						killShotCount[queenColor] += .16666;
						continue;
					}
				} 
			}
			killShotCount[queenColor]++;
		}
	}
	return killShotCount;
}

/*
 * Assumes that the player that is maximizing the score
 * is the one that just moved
 */
double KillShotCountWeight::evaluate(MoveInfo move){
	double result;
	Bitboard b(parentGameState->pinned);
	b.notIntersectionWith(pinnedWatchPoints);
	//see if necessary to recalculate weight
	if ( watchPoints.containsAny(move.newPieceLocation) ||
		 watchPoints.containsAny(move.oldPieceLocation) || 
		 unpinnedWatchPoints.containsAny(parentGameState->pinned) ||
		 b.count() ||
		 queenCount < 2)
	{
		auto killShotCounts = recalculate();
		result = killShotCounts[WHITE] - killShotCounts[BLACK];
	} else {
		result = scores[WHITE] - scores[BLACK];
	}

	//initially assumed maximizing player is WHITE
	//correct assumptions if necessary
	if (parentGameState->turnColor == WHITE) result = -result;
	
	return result*multiplier;
}

double PinnedWeight::evaluate(MoveInfo move){
	int result = 0;

	Bitboard whiteQueenPerimeter, blackQueenPerimeter;
	for (auto& piece: parentGameState->queens.splitIntoBitboards()){
		if (parentGameState->findBottomPieceColor(piece) == WHITE) 
			whiteQueenPerimeter = piece.getPerimeter();
		else 
			blackQueenPerimeter = piece.getPerimeter();
	}
	
	for (Bitboard& piece : parentGameState->allPieces.splitIntoBitboards()){
		//don't count piece if not pinned or if on top of hive
		if (parentGameState->upperLevelPieces.containsAny(piece)
			|| !(parentGameState->pinned.containsAny(piece)))  
		{	
			//don't count piece if pinned beside opposing queen
			if (parentGameState->findTopPieceColor(piece) == WHITE
				&& !blackQueenPerimeter.containsAny(piece))  
			{
				result++;
			} else if (parentGameState->findTopPieceColor(piece) == BLACK
				&& !whiteQueenPerimeter.containsAny(piece))  
			{
				result--;
			}
		}
	}

	if (parentGameState->turnColor == WHITE) 
		result = -result;

	return result * multiplier;
}

void PieceCountWeight::initialize(GameState * gameState){
	Weight::initialize(gameState);
	for (int i = PieceColor::WHITE; i < PieceColor::NONE; i++){
		for (auto element : gameState->unusedPieces[i]) {
			pieceCounts[i] = HivePLM[i][element.first] - element.second;
		}
	}
}

double PieceCountWeight::evaluate(MoveInfo move) {
	double result = pieceCounts[WHITE] - pieceCounts[BLACK];
	if (!(move.newPieceLocation == Bitboard()) && move.oldPieceLocation == Bitboard()) {
		if (parentGameState->findTopPieceColor(move.newPieceLocation) == WHITE)
			result++;
		else 
			result--;
	} 
	
	if (parentGameState->turnColor == WHITE) 
		result = -result;
	return result*multiplier;
}

void AntMoveCountWeight::initialize(GameState * gameState){
	Weight::initialize(gameState);

	freeAnts.clear();
	pinnedAnts.clear();
	coveredAnts.clear();
	allAnts.clear();
	for (auto& graph: antMoves) {
		graph.second.destroy();
	}
	antMoves.clear();
	initialMoveCounts.clear();

	//insert properties of the ant for later use
	for (auto& ant : gameState->ants.splitIntoBitboards()){
		allAnts.unionWith(ant);
		bool free = true;
		PieceColor color = gameState->findBottomPieceColor(ant);
		antMoves[ant.hash()] = MoveGraph();
		initialMoveCounts[ant.hash()] = make_pair(color, 0);
		

		if (gameState->pinned.containsAny(ant)){
			pinnedAnts.unionWith(ant);
			free = false;
		}
		if (gameState->upperLevelPieces.containsAny(ant)){
			coveredAnts.unionWith(ant);
			free = false;
		}
		if (free){
			freeAnts.unionWith(ant);
			moveGen.setGeneratingPieceBoard(&ant);
			Bitboard moves = moveGen.getMoves();

			//insert all moves (and initial ant location) for later use
			antMoves[ant.hash()].insert(ant);
			for (auto& move : moves.splitIntoBitboards()){
				antMoves[ant.hash()].insert(move);	
			}
			initialMoveCounts[ant.hash()] =  make_pair(color, 0);
		}
	}
}

double AntMoveCountWeight::evaluate(MoveInfo move){
	finalMoveCounts = initialMoveCounts;

	Bitboard& newPiece = move.newPieceLocation;
	Bitboard& oldPiece = move.oldPieceLocation;

	Bitboard incorrectAnts(freeAnts);
	Bitboard expectedMoves;

	if (move.pieceName == ANT)
		if (!(move.oldPieceLocation == Bitboard())) 
			expectedMoves = antMoves[move.oldPieceLocation.hash()].getMoves();

	//ant may have been moved to a square it could
	//not previously travel to itself, (ie. spawned or swapped by pillbug)
	//check to see if that is the case
	if (expectedMoves.containsAny(newPiece)) {
		incorrectAnts.notIntersectionWith(oldPiece);
		int newMoves = recalculateMoves(newPiece);
		PieceColor color = parentGameState->findBottomPieceColor(newPiece);
		finalMoveCounts.erase(oldPiece.hash());
		finalMoveCounts[newPiece.hash()] = make_pair(color, newMoves);
	} 

	//see if moving this piece unpinned some ants
	Bitboard newlyUnpinnedAnts(pinnedAnts);
	newlyUnpinnedAnts.unionWith(coveredAnts);
	newlyUnpinnedAnts.notIntersectionWith(parentGameState->upperLevelPieces);
	newlyUnpinnedAnts.notIntersectionWith(parentGameState->pinned);
	
	//recalculate unpinned ants (if any)
	for (auto& unpinned: newlyUnpinnedAnts.splitIntoBitboards()) {
		int newMoves = recalculateMoves(unpinned);
		PieceColor color = parentGameState->findBottomPieceColor(unpinned);
		finalMoveCounts[unpinned.hash()] = make_pair(color, newMoves);
	}

	//see if free ant was made immobile by covering it
	if (freeAnts.containsAny(newPiece)){
		Bitboard& ant = newPiece;
		PieceColor color = parentGameState->findBottomPieceColor(ant);
		finalMoveCounts[ant.hash()] = make_pair(color, 0);
	}

	correctAssumptions(move, incorrectAnts);
	return calculateScore();
}

void AntMoveCountWeight::correctAssumptions(MoveInfo move, Bitboard incorrectAnts){
	for (auto& ant : incorrectAnts.splitIntoBitboards() ){
		
		PieceColor color = parentGameState->findTopPieceColor(ant);
		Bitboard newMoves = antMoves[ant.hash()].getMoves();
		newMoves.notIntersectionWith(move.newPieceLocation);
		moveGen.piecesExceptCurrent = newMoves;

		//find ant paths around the new piece location
		Bitboard legalNodes = moveGen.getLegalWalkPerimeter(move.newPieceLocation);
		Bitboard expectedNodes = move.newPieceLocation.getPerimeter();
		expectedNodes.intersectionWith(newMoves);

		//TODO: case analysis to get faster average case recalculation using articulation
		//nodes and low-link in MoveGraph
		//if the piece does not have an easy recalculation
		if (legalNodes.splitIntoConnectedComponents().size() > 1 ||
			!legalNodes.containsAll(expectedNodes)){

			//brute force recalculate	
			int newMoveCount = recalculateMoves(ant);
			finalMoveCounts[ant.hash()] = make_pair(color, newMoveCount);
			continue;

		} else if (expectedNodes.containsAny(legalNodes)) {
			newMoves.unionWith(legalNodes);
		}


		//recalulate ant paths around the old piece location
		legalNodes = moveGen.getLegalWalkPerimeter(move.oldPieceLocation);
		if (newMoves.containsAny(legalNodes))
			newMoves.unionWith(move.oldPieceLocation);

		//find nodes that are now connected to previous ant moves
		expectedNodes = move.oldPieceLocation.getPerimeter();
		expectedNodes.intersectionWith(newMoves);
		expectedNodes.notIntersectionWith(legalNodes);

		//if there are nodes that were not previously connected
		//O(1) average case recalculation to find new ant moves
		for (auto& startNode: expectedNodes.splitIntoBitboards()) {
			newMoves.notIntersectionWith(startNode);
			moveGen.setGeneratingPieceBoard(&startNode);
			moveGen.findAntMoves(startNode, newMoves);
		}

		finalMoveCounts[ant.hash()] = make_pair(color, newMoves.count());
	}
}

void LadybugMoveCountWeight::initialize(GameState * gameState) {
	Weight::initialize(gameState);
	freeLadybugs.clear();
	upperLevelGates.clear();
	immobileLadybugs.clear();

	ladybugMoves = {Bitboard(), Bitboard()};
	moveGraphs = {IntermediateGraph(), IntermediateGraph()};

	for (auto& ladybug : gameState->ladybugs.splitIntoBitboards()) {
		immobileLadybugs.unionWith(ladybug);
		//if pinned or covered, ignore
		if (gameState->pinned.containsAny(ladybug)) continue;
		if (gameState->upperLevelPieces.containsAny(ladybug)) continue;
		freeLadybugs.unionWith(ladybug);
		immobileLadybugs.xorWith(ladybug);

		//cache moves and extra information for faster recalulation
		moveGen.setGeneratingPieceBoard(&ladybug);
		PieceColor color = gameState->findTopPieceColor(ladybug);
		ladybugMoves[color] = moveGen.getMoves();
		moveGraphs[color] = moveGen.graph;
		upperLevelGates.unionWith(moveGen.extraInfo);
	}
}

double LadybugMoveCountWeight::evaluate(MoveInfo move) {
	finalMoveCounts.clear();
	finalMoveCounts[moveGraphs[WHITE].root.hash()] = {WHITE, ladybugMoves[WHITE].count()};
	finalMoveCounts[moveGraphs[BLACK].root.hash()] = {BLACK, ladybugMoves[BLACK].count()};

	Bitboard& newPiece = move.newPieceLocation;
	Bitboard& oldPiece = move.oldPieceLocation;
	//check if ladybug might be affected by upper level gates
	for (int color = PieceColor::WHITE; color < PieceColor::NONE ; color++){
		Bitboard watched = parentGameState->upperLevelPieces;
		Bitboard intermediate = moveGraphs[color].getIntermediateNodes();
		intermediate.intersectionWith(parentGameState->allPieces);
		
		watched.unionWith(intermediate);
		watched.notIntersectionWith(ladybugMoves[color]);
		watched.intersectionWith(parentGameState->upperLevelPieces);
		if(watched.count() >= 2){
			Bitboard& ladybug = moveGraphs[color].root;
			int moves = recalculateMoves(ladybug);
			finalMoveCounts[ladybug.hash()] = {WHITE, moves};
		}
	}
	//check if a ladybug moves to a different position
	//or was spawned into the board
	if (move.pieceName == LADYBUG && freeLadybugs.containsAny(move.oldPieceLocation)){
		int moves = recalculateMoves(newPiece);
		PieceColor color = parentGameState->findBottomPieceColor(newPiece);
		finalMoveCounts.erase(oldPiece.hash());
		finalMoveCounts[newPiece.hash()] = {color, moves};
	} 

	//check if a previously free ladybug becomes pinned
 	for (auto& ladybug: freeLadybugs.splitIntoBitboards()){
		if (parentGameState->pinned.containsAny(ladybug)){
			PieceColor color = parentGameState->findBottomPieceColor(ladybug);
			finalMoveCounts[ladybug.hash()] = {color, 0};
		}
	}
	
	//check if previously pinned/covered piece is now unpinned and covered
	for (auto& ladybug: immobileLadybugs.splitIntoBitboards()){
		recalculateMoves(ladybug);
		int moves = recalculateMoves(ladybug);
		PieceColor color = parentGameState->findBottomPieceColor(ladybug);
		finalMoveCounts[ladybug.hash()] = {color, moves};
	}

	//if other piece moves in a way that makes previous calculations incorrect
	for (int color = PieceColor::WHITE; color < PieceColor::NONE ; color++){
		auto& graph = moveGraphs[color];
		auto newPiece = move.newPieceLocation;
		auto oldPiece = move.oldPieceLocation;
		newPiece.notIntersectionWith(parentGameState->upperLevelPieces);
		oldPiece.notIntersectionWith(parentGameState->upperLevelPieces);


		Bitboard& ladybug = moveGraphs[color].root;
		int newMoveCount = finalMoveCounts[ladybug.hash()].second;
		vector<int> depths = graph.find(newPiece);
		//if moving to a location where ladybug movement occured
		if (std::find(depths.begin(), depths.end(), 2) != depths.end())
			newMoveCount -= 1;

		//if moving to a location where ladybug movement is probable
		Bitboard newLadybugMoves;
		for (int depth = 0 ; depth < 2 ; depth++ ) { 
			if (std::find(depths.begin(), depths.end(), depth) != depths.end()){
				//find new ladybug moves using limited depth search
				moveGen.generateLadybugMoves(depth);
				newLadybugMoves = moveGen.getMoves();
				//only get unique new moves
				newLadybugMoves.notIntersectionWith(ladybugMoves[color]);
			}
		}
		newMoveCount += newLadybugMoves.count();

		depths = graph.find(oldPiece);
		//if moving away from location that exposes a new move
		if (std::find(depths.begin(), depths.end(), 2) != depths.end()){
			newMoveCount += 1;
		}

		//if moving away from location where some ladybug paths travel through
		if (depths.size() > 0){
			Bitboard dependencies = getDependencies(oldPiece, (PieceColor)color);
			newMoveCount -= dependencies.count();
		}
		finalMoveCounts[ladybug.hash()] = {(PieceColor)color, newMoveCount};
	}
	return calculateScore();
}
//see if any of the given nodes is needed to access a move in ladybugMoves
//if so return said moves
Bitboard checkDependency(Bitboard& nodes, 
						 MoveGenerator& moveGen,
						 IntermediateGraph& graph,
						 int targetNum){
	Bitboard dependent;
	for (auto& node : nodes.splitIntoBitboards()) {
		Bitboard neighbors = moveGen.getLegalClimbs(node);
		vector<int>depths = graph.find(neighbors);
		if ( std::find(depths.begin(), depths.end(), targetNum) == depths.end()) {
			dependent.unionWith(node);
		}
	}
	return dependent;
}

//make sure every move still has a valid edge from depths 0->1->2 
//after removing startNode
//else, list it as a dependency and return
Bitboard LadybugMoveCountWeight::getDependencies(Bitboard startNode, PieceColor color) {
	Bitboard dependentPaths;

	int maxDepth = 2;
	//iterate through the depths potentially dependent on the startNode
	auto graph = moveGraphs[color];
	vector<int> depths = graph.find(startNode);
	graph.remove(startNode);

	for (int depth : depths) {
		Bitboard nodes(startNode);
		while (depth < maxDepth) {
			//using depth count, find any node whose depth depends on solely
			//the specified nodes and remove from graph
			for (auto& node: nodes.splitIntoBitboards()){
				Bitboard testNeighbors = moveGen.getLegalClimbs(node);
				Bitboard dependent = checkDependency(testNeighbors, moveGen, graph, depth);
				nodes.unionWith(dependent);
				dependentPaths.unionWith(dependent);
				graph.remove(dependent);
			}
			depth++;
		}
	}
	dependentPaths.intersectionWith(ladybugMoves[color]);
	return dependentPaths;
}

void GrasshopperMoveCountWeight::initialize(GameState * gameState) {
	Weight::initialize(gameState);
	freeGrasshoppers.clear();
	immobileGrasshoppers.clear();
	watchPoints.clear();
	initialMoveCounts.clear();
	for (auto& grasshopper: gameState->grasshoppers.splitIntoBitboards()){
		Bitboard perimeter;
		PieceColor color = gameState->findBottomPieceColor(grasshopper);

		if (!parentGameState->pinned.containsAny(grasshopper) &&
			!parentGameState->upperLevelPieces.containsAny(grasshopper)) {
			freeGrasshoppers.unionWith(grasshopper);

			//calculate moves by just looking at the perimeter
			perimeter = grasshopper.getPerimeter();
			perimeter.intersectionWith(gameState->allPieces);

		} else {
			immobileGrasshoppers.unionWith(grasshopper);
		}
		grasshopperMoves[grasshopper.hash()] = perimeter.count();
		initialMoveCounts[grasshopper.hash()] = {color, perimeter.count()};
	}
	Bitboard p = freeGrasshoppers.getPerimeter();
	watchPoints.initializeTo(p);
}
double GrasshopperMoveCountWeight::evaluate(MoveInfo move) {
	finalMoveCounts = initialMoveCounts;
	Bitboard& newPiece = move.newPieceLocation;

	Bitboard newlyImmobileGrasshoppers = parentGameState->pinned;
	newlyImmobileGrasshoppers.intersectionWith(freeGrasshoppers);
	newlyImmobileGrasshoppers.notIntersectionWith(newPiece);

	for (auto& grasshopper: newlyImmobileGrasshoppers.splitIntoBitboards()){ 
		PieceColor color  = parentGameState->findBottomPieceColor(grasshopper);
		finalMoveCounts[grasshopper.hash()] = {color, 0};
	}
	
	Bitboard mobileGrasshoppers = immobileGrasshoppers;
	if (move.pieceName == GRASSHOPPER){
		mobileGrasshoppers.unionWith(newPiece);
		finalMoveCounts.erase(newPiece.hash());
	}
	mobileGrasshoppers.notIntersectionWith(parentGameState->pinned);
	mobileGrasshoppers.notIntersectionWith(parentGameState->upperLevelPieces);
	for (auto& grasshopper: mobileGrasshoppers.splitIntoBitboards()){
		PieceColor color = parentGameState->findBottomPieceColor(grasshopper);
		Bitboard perimeter = grasshopper.getPerimeter();
		perimeter.intersectionWith(parentGameState->allPieces);
		finalMoveCounts[grasshopper.hash()] = {color,  perimeter.count()};
	}
	return calculateScore();
}

double SimpleMoveCountWeight::evaluate(MoveInfo move) {
	for (auto& piece: parentGameState->getPieces(name)-> splitIntoBitboards()){
		PieceColor color = parentGameState->findTopPieceColor(piece);
		finalMoveCounts[piece.hash()] = {color, recalculateMoves(piece)};
	}
	return calculateScore();
}

double MosquitoMoveCountWeight::evaluate(MoveInfo move){
	vector<int> results {0,0};
	for (auto& mosquito: parentGameState->mosquitoes.splitIntoBitboards()){
		if (parentGameState->findTopPieceName(mosquito) != MOSQUITO) {
			continue;
		}
		PieceColor color = parentGameState->findTopPieceColor(mosquito);
		if (parentGameState->upperLevelPieces.containsAny(mosquito)){
			PieceName name = BEETLE;
			moveGen.setGeneratingName(&name);
			Bitboard moves = moveGen.getMoves();
			results[color] = moves.count();
			continue;
		}
		Bitboard inheritable = mosquito.getPerimeter();
		inheritable.notIntersectionWith(parentGameState->mosquitoes);
		for (auto piece : inheritable.splitIntoBitboards()){
			for (auto w : weights) {
				if (w->finalMoveCounts.find(piece.hash()) != w->finalMoveCounts.end() ){
					results[color] = w->finalMoveCounts[piece.hash()].second;
					break;
				}
			}
		}
	}
	double result = results[WHITE] - results[BLACK];
	if (parentGameState-> turnColor == WHITE) {
		result = -result;
	}
	return result *multiplier;
}

