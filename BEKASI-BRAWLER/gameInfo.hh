#include <set>
#include <map>
#include <iostream>

using namespace std;

struct Cell {
  int x, y;
  Cell() {}
  Cell(int x, int y);
  Cell(istream &in);
  bool operator<(const Cell &another) const;
  bool operator==(const Cell &another) const;
};

ostream &operator<<(ostream &out, const Cell &cell);

struct GameInfo {
  int id;
  int size;
  int step;
  int maxSteps;
  set <Cell> holes;
  map <Cell, int> revealedTreasure;
  map <Cell, int> sensedTreasure;
  Cell positions[4];
  int plans[4];
  int actions[4];
  int scores[2];
  int remaining;
  int thinkTime;
  GameInfo(istream &in);
};

ostream &operator<<(ostream &out, const GameInfo &gameinfo);
