#include "players.hh"
#include <algorithm>
#include <unordered_set>
#include <queue>

map<Cell, int> countVisit;
int failedPlanCounter;
int dogStandStillCounter;
set <Cell> forbiddenCells;
bool treasureMatrix[100][100] = {false};

int planSamurai(GameInfo &info) {
  if (info.step == 0) initFieldMap(info);
  int id = info.id;
  Cell pos = info.positions[id];
  CellInfo &myCell = cells[pos.x][pos.y];

  if(countVisit.find(pos) != countVisit.end()){
    int countDtl = countVisit.find(pos)->second;
    countVisit.erase(pos);
    countDtl++;
    if(countDtl > 2 && forbiddenCells.count(pos) == 0){
      forbiddenCells.insert(pos);
    } 
    countVisit.insert({pos, countDtl});

  } else {
    countVisit.insert({pos, 1});
  }

  Cell dogPos = info.positions[id+2];
  CellInfo &dogPosInfo = cells[dogPos.x][dogPos.y];
  CellInfo* dogPosInforPtr = &dogPosInfo;

  Cell enemyPos = id == 0 ? info.positions[1] : info.positions[0];
  CellInfo &enemyPosInfo = cells[enemyPos.x][enemyPos.y];

  bool dogIsClose = std::find(myCell.eightNeighbors.begin(), myCell.eightNeighbors.end(), dogPosInforPtr) != myCell.eightNeighbors.end();

  int avoidPlan = -2;
  //Repeat failed plan
  if (info.step > 1 && info.plans[id] != info.actions[id]) {
    failedPlanCounter++;

    if(dogIsClose || forbiddenCells.count(reverseDirectionOf(pos, info.plans[id])) != 0 || failedPlanCounter > 1) {
      failedPlanCounter = 0;
      avoidPlan = info.plans[id];
    } else {
      return info.plans[id];
    }
  }

  failedPlanCounter = 0;

  // Dig adjacent known treasure
  Cell digCand;
  int maxTreasure = -1;
  for (auto &n: myCell.fourNeighbors) {
    if (noAgentsIn(n->position, info) || (dogPos == n->position)) {
      auto treasure = info.revealedTreasure.find(n->position);
      if (treasure != info.revealedTreasure.end() && treasure->second > maxTreasure) { // second = value nya
	      digCand = n->position;
	      maxTreasure = treasure->second;
      }
    }
  }
  if (maxTreasure > 0) {
    int plan = directionOf(pos, digCand) + 8;
    if(plan != avoidPlan) {
      return plan;
    }
  }
  // Move towards the nearest known treasure
  {
    vector <pair<Cell, int>> candidates;
    int closest = numeric_limits<int>::max();
    int playerToEnemyDist = customSamuraiDistance(&myCell, &enemyPosInfo, info.holes);

    // if a bunch of treasures are close, aim for the center cell also
    map<Cell, int> extendedTreasures;
    for (auto g: info.revealedTreasure) {
      CellInfo &t = cells[g.first.x][g.first.y];
      
      if (!treasureMatrix[g.first.x][g.first.y]) {
        treasureMatrix[g.first.x][g.first.y] = true;
      }
      
      for (auto gn: t.eightNeighbors){
        auto treasure = info.revealedTreasure.find(gn->position);
        
        if (treasure != info.revealedTreasure.end() ) {
          int dirBetweenTreasure = directionOf(t.position, gn->position);

          if (dirBetweenTreasure % 2 != 0) {
            CellInfo &firstCenter = cells[t.position.x][gn->position.y];
            CellInfo &secondCenter = cells[gn->position.x][t.position.y]; 
            extendedTreasures[firstCenter.position] = g.second + treasure->second;
            extendedTreasures[secondCenter.position] = g.second + treasure->second;
          }
        }  
      }
      extendedTreasures[g.first] = g.second;
    }

    //Calculate graph-view of enemy's best possible path to collect treasure
    set<Cell> revealedTreasureSet;
    for (auto t: info.revealedTreasure) {
      revealedTreasureSet.insert(t.first);      
    }

    map<Cell, int> enemyDistanceToTreasure;
    pair<Cell, int> smallestDistanceForEnemy = pair<Cell, int>(pos, numeric_limits<int>::max());
    CellInfo* enemyStartPoint = &enemyPosInfo;

    while (!revealedTreasureSet.empty()) {
      for(auto t: revealedTreasureSet) {
        CellInfo* treasureInfo = &cells[t.x][t.y];
        int enemyDist = customSamuraiDistance(enemyStartPoint, treasureInfo, info.holes);
        if(enemyDist < smallestDistanceForEnemy.second) {
          smallestDistanceForEnemy.first = t;
          smallestDistanceForEnemy.second = enemyDist;
        }
      }
      if(enemyStartPoint != &enemyPosInfo) {
        int oldDistance = enemyDistanceToTreasure.find(enemyStartPoint->position)->second;
        int newDistance = oldDistance + smallestDistanceForEnemy.second + 2;
        enemyDistanceToTreasure.insert({smallestDistanceForEnemy.first, newDistance});
      } else {
        enemyDistanceToTreasure.insert({smallestDistanceForEnemy.first, smallestDistanceForEnemy.second});
      }

      smallestDistanceForEnemy.second = numeric_limits<int>::max();
      enemyStartPoint = &cells[smallestDistanceForEnemy.first.x][smallestDistanceForEnemy.first.y];
      revealedTreasureSet.erase(smallestDistanceForEnemy.first);
    }

    for (auto g: extendedTreasures) {
      if (!noAgentsIn(g.first, info)) {
        if(dogPos.x != g.first.x && dogPos.y != g.first.y) {
          continue;
        }
      }
      
      for (auto n: myCell.fourNeighbors) { 
        if (noAgentsIn(n->position, info)) {
	        int dist = customSamuraiDistance(n, &cells[g.first.x][g.first.y], info.holes) + 1;
          //Calculate enemy distance to current treasure, skip if enemy is closer0
          int enemyDist = enemyDistanceToTreasure.find(g.first)->second;
          if (enemyDist < dist) {
            continue;
          }

          if (dist <= closest) {
            if (dist != closest) {
	            candidates.clear();
	            closest = dist;
	          }

            //Replace original candidate with candidate that further our distance with enemy
            int newPlayerToEnemyDist = customSamuraiDistance(n, &enemyPosInfo, info.holes);
            if (newPlayerToEnemyDist > playerToEnemyDist && info.revealedTreasure.size() > 1) {
              playerToEnemyDist = newPlayerToEnemyDist;
              candidates.clear();
            }

            if (forbiddenCells.count(n->position) != 0) {
              continue;
            }

	          candidates.push_back(pair<Cell, int>(n->position, g.second)); //TEMP, revert dist to g.second
	        }
	      }
      }
    }

    if (!candidates.empty()) {
      sort(candidates.begin(), candidates.end(),[](auto c1, auto c2) { return c1.second > c2.second; });
      for (auto c: candidates) {
	      int plan = directionOf(pos, c.first) + (info.holes.count(c.first) == 0 ? 0 : 16);
	      if (plan != avoidPlan) return plan;
      }
    }
  }

  //NEW STRATEGY 21/01/2020: RANDOM DIGGING
  for(Cell h: info.holes) {
    if (!treasureMatrix[h.x][h.y]){
      treasureMatrix[h.x][h.y] = true;
    }
    
  }

  for (CellInfo* n: myCell.fourNeighbors) {
    if(!treasureMatrix[n->position.x][n->position.y]) {
      return directionOf(pos, n->position) + 8;
    }
  }


  // No revealed gold
  // Try to approach the SAMURAI, hoping it to find some gold
  int closest = numeric_limits<int>::max();
  int bestPlan = -1;

  if(info.plans[id+2] != info.actions[id+2] && info.plans[id+2] != -1) {
    dogStandStillCounter++;
  } else {
    dogStandStillCounter = 0;
  }

  CellInfo* destinationInfo = &enemyPosInfo;
  if(dogStandStillCounter > 3) {
    destinationInfo = &dogPosInfo;
  }

  for (auto n: myCell.fourNeighbors) {
    if (noAgentsIn(n->position, info)) {
      int dist = customSamuraiDistance(n, destinationInfo, info.holes);
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
