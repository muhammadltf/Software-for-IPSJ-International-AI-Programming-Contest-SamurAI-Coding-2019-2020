#include "players.hh"

const int FailureAvoidPercentage = 70;
const int WaitChanceThresh = -2;
const int StrollDepth = 0;

static int gatheredInfo(int depth, CellInfo &start, const GameInfo &info) {
  int gathered = 0;
  vector <CellInfo *> newInfo;
  for (CellInfo *n: start.eightNeighbors) {
    if (!n->inspected) {
      gathered += 1;
      n->inspected = true;
      newInfo.push_back(n);
    }
  }
  int best = 0;
  if (depth != 0) {
    depth -= 1;
    for (CellInfo *n: start.eightNeighbors) {
      best = max(best, gatheredInfo(depth, *n, info));
    }
  }
  for (CellInfo *n: newInfo) n->inspected = false;
  return gathered + best;
}

int planDog(GameInfo &info) {
  if (info.step == 0) initFieldMap(info);
  int id = info.id;
  int avoidPlan =
    info.plans[id] != info.actions[id] ? // should be avoided
    info.plans[id] : -2;		  // with certain probability
  
  Cell pos = info.positions[id];
  CellInfo &myCell = cells[pos.x][pos.y];

  Cell samuraiPos = info.positions[id - 2];
  CellInfo *samuraiCell = &cells[samuraiPos.x][samuraiPos.y];

  Cell oppPos = info.positions[1 - id%2];
  CellInfo *oppCell = &cells[oppPos.x][oppPos.y];

  vector <CellInfo *> moveCandidates;

  int avoidSamurai[] = {-1, -1};
  for (CellInfo *n: myCell.eightNeighbors) {
    if (n->position == samuraiCell->position){
      int dirToSamurai = directionOf(n->position, samuraiCell->position);
      avoidSamurai[0] = (dirToSamurai+1) % 8;
      avoidSamurai[1] = (dirToSamurai-1) < 0 ? 7 : (dirToSamurai-1); 
    }

    n->inspected = true;
    if (noAgentsIn(n->position, info) && noHolesIn(n->position, info)) {
      moveCandidates.push_back(n);
    }
  }

  if (info.revealedTreasure.find(pos) != info.revealedTreasure.end()) {
    // The dog is in the cell with known gold
    bool friendCanDig = false;
    bool oppCanDig = false;
    for (CellInfo *p: myCell.fourNeighbors) {
      if (p == samuraiCell) friendCanDig = true;
      else if (p == oppCell) oppCanDig = true;
    }
    // If opponent samurai can dig, but friend samurai can't,
    // then sit there still to prevent opponent's digging.
    if (oppCanDig && !friendCanDig) return -1;
  }
  Cell candidate;
  int largestDiff = numeric_limits<int>::min();
  int largestAmount = 0;
  for (auto s: info.sensedTreasure) {
    CellInfo *sensed = &cells[s.first.x][s.first.y];
    int distDiff = 
      customSamuraiDistance(oppCell, sensed, info.holes) -
      customSamuraiDistance(samuraiCell, sensed, info.holes);
    if (distDiff > largestDiff || (distDiff == largestDiff && s.second > largestAmount)) {
      // Check the distances of friend and opponent samurai.
      // Also check if any other agents are in the gold cell.
      if (noAgentsIn(s.first, info)) {
        // If not, going to the cell can be a candidate plan
        largestDiff = distDiff;
        largestAmount = s.second;
        candidate = s.first;
      }
    }
  }
  if (largestDiff >= 0) {
    // If the friend samurai is closer to the treasure cell
    // go there (and bark).
    int plan = directionOf(pos,candidate);
    if (plan != avoidPlan) {
          return plan;
    }
  } else if (largestDiff > WaitChanceThresh) {
    // Else if the distance difference is not large,
    // stand still waiting for a next chance.
    return -1;
  }

  // MOVE TO OPPONENT's CLOSEST GOLD
  int lowestOppToTreasureDist = numeric_limits<int>::max();
  int bestDistance = numeric_limits<int>::max();
  int bestPlan = -1;
  for(auto t: info.revealedTreasure) {
    CellInfo* treasure = &cells[t.first.x][t.first.y];
    int samuraiDist = customSamuraiDistance(samuraiCell, treasure, info.holes);
    int oppToTreasureDist = customSamuraiDistance(oppCell, treasure, info.holes);

    if (oppToTreasureDist <= lowestOppToTreasureDist  && samuraiDist > 2 && !(treasure->position == pos)) {
      lowestOppToTreasureDist = oppToTreasureDist;
      bestDistance = numeric_limits<int>::max();

      for(auto n: myCell.eightNeighbors) {
        int dist = customSamuraiDistance(n, treasure, info.holes) + 1;
        if(dist < bestDistance && noHolesIn(n->position, info)) {
          bestDistance = dist;
          bestPlan = directionOf(myCell.position, n->position);
        }
      }
    }
  }

  if (bestPlan != -1 && bestPlan != avoidPlan && bestPlan != avoidSamurai[0] && bestPlan != avoidSamurai[1]) {
    return bestPlan;
  }

  // Start strolling to gather as much info as possible
  int maxInfo = 0;
  vector <int> bestPlans;
  for (CellInfo *c: moveCandidates) {
    int gathered = gatheredInfo(StrollDepth, *c, info);
    if (gathered >= maxInfo) {
      int plan = directionOf(pos, c->position);
      if (plan != avoidPlan && plan != avoidSamurai[0] && plan != avoidSamurai[1]) {
	      if (gathered > maxInfo) {
	        bestPlans.clear();
	        maxInfo = gathered;
	      }
	      bestPlans.push_back(plan);
      }
    }
  }

  if(bestPlans.empty()) {
    for (auto n: myCell.eightNeighbors){
      int direction = directionOf(myCell.position, n->position);
      bestPlans.push_back(direction);
    }
    
  }

  return bestPlans[rand()%bestPlans.size()];
}
