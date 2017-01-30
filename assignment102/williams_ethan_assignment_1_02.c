#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include "generateDungeon.h"
#include<argp.h>

void readDungeonInfo();
void writeDungeonInfo();

typedef struct{
  int upperX;
  int uperY;
  int width;
  int height;
}Room;

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
    FILE *dungeonInfo = fopen(strcat(path, "/dungeon"), "r");

    readDungeonInfo(dungeonInfo);

    printDungeon(&dungeon);

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
    //Finds total bytes in document for use later
  fseek(dungeonInfo, 0, SEEK_END);
  long int bytes = ftell(dungeonInfo);
  
  //Starts at byte 20 and takes all bytes which denote hardness
  fseek(dungeonInfo, 20, SEEK_SET);
  char buffer[16800];
  fread(buffer, 1, 16800, dungeonInfo);
  int i;
  for(i = 0; i < 16800; i++){
    dungeon.spaces[i/105][i%105].hardness = atoi(buffer[i]);
  }

  //Traverses through document to find room information
  char buffer2[bytes - 16800];
  Room rooms[(bytes - 16800) / 4];
  fread(buffer2, 1, bytes-16800, dungeonInfo);
  int j;
  int k = 0;
  for(j = 0; j < (bytes - 16800) / 4; j++){
    rooms[j].upperX = buffer2[k];
    rooms[j].upperY = buffer2[k+1];
    rooms[j].width = buffer2[k+2];
    rooms[j].height = buffer2[k+3];
    k+=4;
  }

  //Plots rooms in dungeon based on info read
  
  //If hardness is zero and not a room, changes to corridor. Otherwise, sets material to rock
  
}
void writeDungeonInfo(){

}
