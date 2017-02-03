#ifndef DISK_OPERATIONS_H
#define DISK_OPERATIONS_H

void readDungeonInfo();
void writeDungeonInfo();
void saveload(Dungeon *dungeon, int *numRooms, char *arg1, char *arg2);

typedef struct{
  int upperX;
  int upperY;
  int width;
  int height;
}Room;

#endif
