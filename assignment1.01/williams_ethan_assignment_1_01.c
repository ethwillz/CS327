#include "generateDungeon.h"

int main(int argc, char* argv[]){
  //Generates 18 rooms randomly
  generateRooms(argv[1]);

  //Prints dungeon
  printDungeon();

  return 0;
}
