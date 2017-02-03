#include <stdio.h>
#include "generateDungeon.h"
#include "diskOperations.h"

int main(int argc, char *argv[]){
  Dungeon dungeon;

  int numRooms;

  saveload(&dungeon, &numRooms, argv[1], argv[2]);

  printf("%d\n", numRooms);

  //Choose a random location in the room to place PC

  //Calculate distances for non-tunneling monsters

  //Calculate distances for tunneling monsters
}
