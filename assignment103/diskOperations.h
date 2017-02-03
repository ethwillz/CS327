#ifndef DISK_OPERATIONS_H
#define DISK_OPERATIONS_H

typedef struct{
  int upperX;
  int upperY;
  int width;
  int height;
}Room;

typedef struct{
  Room *allRooms;
}Rooms;

void readDungeonInfo();
void writeDungeonInfo();
void saveload(Dungeon *dungeon, Rooms *rooms, char *arg1, char *arg2);

#endif
