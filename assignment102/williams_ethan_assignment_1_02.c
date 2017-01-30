#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include "generateDungeon.h"
#include<argp.h>

void readDungeonInfo();
void writeDungeonInfo();

int main(int argc, char *argv[]){

  Dungeon dungeon;

 mkdir("/home/student/.rlg327/", 0777);

  //Checks if desired directory exists, creates directory if not
  char *dir = getenv("HOME");
  char *path = strcat(dir, "/.rlg327/");
  if(path == NULL)
    mkdir("/home/student/.rlg327/", 0777);

  if(argv[1] == NULL){
    generateRooms(argv[1], &dungeon);
    printDungeon(&dungeon);
  }
  //If user adds load switch
  if(strcmp(argv[1], "--LOAD") == 0){
    FILE *dungeonInfo = fopen(strcat(path, "/dungeon.rlg327"), "r");

    readDungeonInfo(dungeonInfo);

    fclose(dungeonInfo);
  }
  //If user adds save switch
  else if(strcmp(argv[1], "--SAVE") == 0){
    FILE *dungeonInfo = fopen(path, "w");

    generateRooms(argv[1], &dungeon);
    writeDungeonInfo();
    printDungeon(&dungeon);

    fclose(dungeonInfo);
  }
  //If user adds an unrecognized switch/input, displays random dungeon and exits
  else{
    generateRooms(argv[1], &dungeon);
    printDungeon(&dungeon);
  }
  
  return 0;
}

void readDungeonInfo(FILE *dungeonInfo){
  int i;
  fseek(dungeonInfo, 0L, SEE_END);
  int bytes = ftell(*dungeonInfo);
  printf("%d", bytes);
}
void writeDungeonInfo(){

}
