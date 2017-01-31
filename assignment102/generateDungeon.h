#ifndef GENERATE_DUNGEON_H
#define GENERATE_DUNGEON_H

struct room{
  int x;
  int y;
};

typedef struct{
  char material;
  int  hardness;
}Space;

typedef struct{
  Space spaces[105][160];
}Dungeon;

void generateRooms(Dungeon *dungeon);

void connect(int num, Dungeon *dungeon);

void insertLeft(int *x, int *y, Dungeon *dungeon);

void insertRight(int *x, int *y, Dungeon *dungeon);

void insertUp(int *x, int *y, Dungeon *dungeon);

void insertDown(int *x, int *y, Dungeon *dungeon);

void printDungeon(Dungeon *dungeon);

#endif
