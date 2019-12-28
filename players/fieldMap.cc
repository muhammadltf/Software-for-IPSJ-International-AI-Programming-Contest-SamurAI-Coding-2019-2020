#include "fieldMap.hh"
#include <unordered_set>

CellInfo **cells;

void initFieldMap(GameInfo &info) {
  cells = new CellInfo*[info.size];
  for (int x = 0; x != info.size; x++) {
    cells[x] = new CellInfo[info.size];
  }
  static const int dx[] = { 0,-1,-1,-1, 0, 1, 1, 1 };
  static const int dy[] = { 1, 1, 0,-1,-1,-1, 0, 1 };
  for (int x = 0; x != info.size; x++) {
    for (int y = 0; y != info.size; y++) {
      CellInfo &c = cells[x][y];
      c.position = Cell(x, y);
      for (int k = 0; k != 8; k++) {
	int nx = x + dx[k];
	int ny = y + dy[k];
	if (0 <= nx && nx < info.size &&
	    0 <= ny && ny < info.size) {
	  c.eightNeighbors.push_back(&cells[nx][ny]);
	  if (k%2 == 0) c.fourNeighbors.push_back(&cells[nx][ny]);
	}
      }
    }
  }
}

int samuraiDistance(CellInfo *from, CellInfo *to, set <Cell> &holes) {
  stack <CellInfo*> next0;
  stack <CellInfo*> next1;
  stack <CellInfo*> next2;
  next0.push(from);
  stack <CellInfo*> *np0 = &next0;
  stack <CellInfo*> *np1 = &next1;
  stack <CellInfo*> *np2 = &next2;
  unordered_set <CellInfo*> visited;
  for (int dist = 0; ; dist++) {
    while (!np0->empty()) {
      CellInfo *c = np0->top(); np0->pop();
      if (visited.count(c) == 0) {
	      visited.insert(c);
	      for (auto n: c->fourNeighbors) {
	        if (n == to) return dist;
	        if (holes.count(n->position) == 0) {
	          np1->push(n);
	        } else {
	          np2->push(n);
	        }
	      }
      }
    }
    np0 = np1; np1 = np2; np2 = np0;
  }
}

int customSamuraiDistance(CellInfo *from, CellInfo *to, set <Cell> &holes, unordered_set <CellInfo*> visit) {
  stack <pair<CellInfo*, int>> next0;
  if (holes.count(from->position) != 0) {
    next0.push(pair<CellInfo*, int>(from, 1));
  } else {
    next0.push(pair<CellInfo*, int>(from, 0));
  }
  stack <pair<CellInfo*, int>> *np0 = &next0;

  int lowX = from->position.x < to->position.x ? from->position.x : to->position.x;
  int highX = from->position.x > to->position.x ? from->position.x : to->position.x;
  int lowY = from->position.y < to->position.y ? from->position.y : to->position.y;
  int highY = from->position.y > to->position.y ? from->position.y : to->position.y;

  int bestDistance = numeric_limits<int>::max();

  while (!np0->empty()) {
    pair<CellInfo*, int> c = np0->top(); np0->pop();
    // std::cerr << "ISI STACK: " + to_string(c.first->position.x) + "," + to_string(c.first->position.y) + " | DISTANCE: " + to_string(c.second) << endl;
    if (c.second >= bestDistance) {
      continue;
    }

    visit.insert(c.first);
    if (c.first->position == to->position) {
      // std::cerr << "KETEMU!! : " + to_string(c.first->position.x) + "," + to_string(c.first->position.y) + " | DISTANCE: " + to_string(c.second) << endl;
      if (c.second < bestDistance) {
        bestDistance = c.second;
      }
      continue;
    }

    int currentDistance = c.second;
    currentDistance++;

    for (auto n: c.first->fourNeighbors){
      if (visit.count(n) != 0 || n->position.x < lowX || n->position.x > highX || n->position.y < lowY || n->position.y > highY) {
        // std::cerr << "VISITED!!" << endl;
        continue;
      } else {
        // std::cerr << "BELOM VISIT DONG!!" << endl;
        if (holes.count(n->position) != 0) {
          currentDistance++;
        } 

        np0->push(pair<CellInfo*, int>(n, currentDistance));
      }
    }
  }

  // std::cerr << "TITIK EKSTRIM (LOWX, HIGHX, LOWY, HIGHY): " + to_string(lowX) + "," + to_string(highX) + "," + to_string(lowY) + "," + to_string(highY) << endl;
  // std::cerr << "DARI: " + to_string(from->position.x) + "," + to_string(from->position.y) + " | KE: " + to_string(to->position.x) + "," + to_string(to->position.y) << endl;
  // std::cerr << "BEST DISTANCE: " + to_string(bestDistance) << endl;
  return bestDistance;
}
