#ifndef GENERATE_DUNGEON_H
#define GENERATE_DUNGEON_H

struct room{
  int x;
  int y;
};

struct space{
  char material;
  int  hardness;
};

void generateRooms(char *userInput);

void connect(int num);

void insertLeft(int *x, int *y);

void insertRight(int *x, int *y);

void insertUp(int *x, int *y);

void insertDown(int *x, int *y);

void printDungeon();

#endif
