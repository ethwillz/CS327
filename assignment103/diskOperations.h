#ifndef DISK_OPERATIONS_H
#define DISK_OPERATIONS_H

typedef struct{
  int upperX;
  int upperY;
  int width;
  int height;
}Room;

void readDungeonInfo();
void writeDungeonInfo();
void saveload(Dungeon *dungeon, char *arg1, char *arg2);

#endif
