#include "players.hh"
#include <algorithm>
#include <unordered_set>

int planSamurai(GameInfo &info) {
  if (info.step == 0) initFieldMap(info);
  int id = info.id;
  Cell pos = info.positions[id];
  CellInfo &myCell = cells[pos.x][pos.y];

  Cell dogPos = info.positions[id+2];
  CellInfo &dogPosInfo = cells[dogPos.x][dogPos.y];
  CellInfo* dogPosInforPtr = &dogPosInfo;

  bool dogIsClose = std::find(myCell.eightNeighbors.begin(), myCell.eightNeighbors.end(), dogPosInforPtr) != myCell.eightNeighbors.end();

  int avoidPlan = -2;
  //Repeat failed plan
  if (info.step > 1 && info.plans[id] != info.actions[id]) {
    if(dogIsClose) {
      avoidPlan = info.plans[id];
    } else {
      return info.plans[id];
    }
  }

  // Dig adjacent known treasure
  Cell digCand;
  int maxTreasure = -1;
  for (auto &n: myCell.fourNeighbors) {
    if (noAgentsIn(n->position, info)) {
      auto treasure = info.revealedTreasure.find(n->position);
      if (treasure != info.revealedTreasure.end() && treasure->second > maxTreasure) { // second = value nya
	      digCand = n->position;
	      maxTreasure = treasure->second;
      }
    }
  }
  if (maxTreasure > 0) return directionOf(pos, digCand) + 8;
  // Move towards the nearest known treasure
  {
    vector <pair<Cell, int>> candidates;
    int closest = numeric_limits<int>::max();
    Cell enemyPos = id == 0 ? info.positions[1] : info.positions[0];
    CellInfo &enemyPosInfo = cells[enemyPos.x][enemyPos.y];
    int playerToEnemyDist = customSamuraiDistance(&myCell, &enemyPosInfo, info.holes, unordered_set < CellInfo*> ());

    // if a bunch of treasures are close, aim for the center cell also
    map<Cell, int> extendedTreasures;
    for (auto g: info.revealedTreasure) {
      CellInfo &t = cells[g.first.x][g.first.y];
      
      for (auto gn: t.eightNeighbors){
        auto treasure = info.revealedTreasure.find(gn->position);
        
        if (treasure != info.revealedTreasure.end() ) { // second = value nya
          int dirBetweenTreasure = directionOf(t.position, gn->position);

          if (dirBetweenTreasure % 2 != 0) {
            CellInfo &firstCenter = cells[t.position.x][gn->position.y];
            CellInfo &secondCenter = cells[t.position.y][gn->position.x]; 
            extendedTreasures[firstCenter.position] = g.second + treasure->second;
            extendedTreasures[secondCenter.position] = g.second + treasure->second;
          }
        }  
      }
      extendedTreasures[g.first] = g.second;
    }

    //Go to the next best treasure
    // bool alreadySkipTreasure = false;
    pair<Cell, int> localClosest;
    int oldDist = numeric_limits<int>::max();

    for (auto g: extendedTreasures) {
      for (auto n: myCell.fourNeighbors) { 
        if (noAgentsIn(n->position, info)) {
          unordered_set <CellInfo*> visited;
	        int dist = customSamuraiDistance(n, &cells[g.first.x][g.first.y], info.holes, visited) + 1;
              
          //Calculate enemy distance to current treasure, skip if enemy is closer
          int enemyDist = customSamuraiDistance(&enemyPosInfo, &cells[g.first.x][g.first.y], info.holes, visited);
          // std::cerr << "JARAK ENEMY! (" + to_string(enemyPosInfo.position.x) + "," + to_string(enemyPosInfo.position.y) + ") distance: " + to_string(enemyDist) << endl;
          if (enemyDist < dist) {
            if (dist < oldDist){
              localClosest = pair<Cell, int>(n->position, g.second);
              oldDist = dist;
            }

            std::cerr << "ENEMY IS CLOSER! Skip treasure (" + to_string(g.first.x) + "," + to_string(g.first.y) + ") distance: " + to_string(dist) + " vs " + to_string(enemyDist) + " ; " + to_string(n->position.x) + "," + to_string(n->position.y) << endl;
            continue;
          }

          if (dist <= closest) {
            if (dist != closest) {
	            candidates.clear();
	            closest = dist;
	          }

            //Replace original candidate with candidate that further our distance with enemy
            int newPlayerToEnemyDist = customSamuraiDistance(n, &enemyPosInfo, info.holes, visited);
            if (newPlayerToEnemyDist > playerToEnemyDist && info.revealedTreasure.size() > 1) {
              std::cerr << "Move away from enemy! Original dist= " + to_string(playerToEnemyDist) + ", New dist= " + to_string(newPlayerToEnemyDist) << endl;
              playerToEnemyDist = newPlayerToEnemyDist;
              candidates.clear();
            }

            std::cerr << to_string(g.first.x) + "," + to_string(g.first.y) + ";" + to_string(n->position.x) + "," + to_string(n->position.y) + ";" + to_string(dist) << endl;
	          candidates.push_back(pair<Cell, int>(n->position, g.second)); //TEMP, revert dist to g.second
	        }
	      }
      }
      if(candidates.empty()){
        closest = numeric_limits<int>::max();
        candidates.push_back(localClosest);
      }

      //TODO kalo menang semua, urut dari yang paling gede, adu sama jalur lawan

      std::cerr << "" << endl;
    }

    if (!candidates.empty()) {
      sort(candidates.begin(), candidates.end(),[](auto c1, auto c2) { return c1.second > c2.second; });
      for (auto c: candidates) {
        std::cerr << "EXECUTE: " + to_string(c.first.x) + "," + to_string(c.first.y) << endl;
        std::cerr << "DIRECTION: " + to_string(directionOf(pos, c.first)) << endl;
        std::cerr << "AVOID: " + to_string(avoidPlan) << endl;
        std::cerr << "" << endl;
	      int plan = directionOf(pos, c.first) + (info.holes.count(c.first) == 0 ? 0 : 16);
	      if (plan != avoidPlan) return plan;
      }
    }
  }
  // No revealed gold
  // Try to approach the dog, hoping it to find some gold
  int closest = numeric_limits<int>::max();
  int bestPlan = -1;
  for (auto n: myCell.fourNeighbors) {
    if (noAgentsIn(n->position, info)) {
      int dist = samuraiDistance(n, &dogPosInfo, info.holes);
      if (dist < closest) {
	      int plan  = directionOf(pos, n->position) + (info.holes.count(n->position) == 0 ? 0 : 16);
        if (plan != avoidPlan) {
	        bestPlan = plan;
	        closest = dist;
	      }
      }
    }
  }
  return bestPlan;
}
