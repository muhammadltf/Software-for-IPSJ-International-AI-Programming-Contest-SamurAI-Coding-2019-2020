#include "players.hh"

const int digPercentage = 80;

int planSamurai(GameInfo &info) {
  if (info.step == 0) initFieldMap(info);
  Cell pos = info.positions[info.id];
  CellInfo myCell = cells[pos.x][pos.y];
  int plan = -1;
  int trials = 10;
  while (--trials > 0) {
    auto n = myCell.fourNeighbors[rand()%myCell.fourNeighbors.size()];
    int dir = directionOf(myCell.position, n->position);
    bool noHole = noHolesIn(n->position, info);
    if (noAgentsIn(n->position, info)) {
      if (rand()%100 < digPercentage && noHole) {
	      plan = dir+8;
	      break;
      } else if (noHole) {
	      plan = dir;
	      break;
      } else {
        plan = dir+16;
        break;
      }
    }
  }
  return plan;
}
