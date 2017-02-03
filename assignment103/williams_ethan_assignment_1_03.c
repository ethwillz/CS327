#include <stdio.h>
#include "generateDungeon.h"
#include "diskOperations.h"

int main(int argc, char *argv[]){
  Dungeon dungeon;

  Rooms rooms;

  saveload(&dungeon, &rooms, argv[1], argv[2]);

  //Choose a random location in the room to place PC

  //Calculate distances for non-tunneling monsters

  //Calculate distances for tunneling monsters

  return 0;
}
