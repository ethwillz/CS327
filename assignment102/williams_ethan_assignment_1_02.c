#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include "generateDungeon.h"
#include <endian.h>
#include <math.h>

void readDungeonInfo();
void writeDungeonInfo();
void findRoom();

typedef struct{
  int upperX;
  int upperY;
  int width;
  int height;
}Room;

int main(int argc, char *argv[]){

  Dungeon dungeon;

  //Checks if desired directory exists, creates directory if not
  char *dir = getenv("HOME");
  char *path = strcat(dir, "/.rlg327/");
  mkdir(path, 0777);

  if(argv[1] == NULL){
    generateRooms(&dungeon);
    printDungeon(&dungeon);
  }
  //If user adds load switch
  else if(strcmp(argv[1], "--LOAD") == 0 && argv[2] != NULL){
    FILE *dungeonInfo = fopen(strcat(path, argv[2]), "r");

    readDungeonInfo(dungeonInfo, &dungeon);

    printDungeon(&dungeon);

    fclose(dungeonInfo);
  }
  else if(strcmp(argv[1], "--LOAD") == 0){
    FILE *dungeonInfo = fopen(strcat(path, "dungeon"), "r");

    readDungeonInfo(dungeonInfo, &dungeon);

    printDungeon(&dungeon);

    fclose(dungeonInfo);
  }
  //If user adds save switch
  else if(strcmp(argv[1], "--SAVE") == 0){
    FILE *dungeonInfo = fopen(strcat(path, "dungeon"), "w");

    generateRooms(&dungeon);
    writeDungeonInfo(dungeonInfo, &dungeon);
    printDungeon(&dungeon);

    fclose(dungeonInfo);
  }
  //If user adds an unrecognized switch/input, displays random dungeon and exits
  else{
    generateRooms(&dungeon);
    printDungeon(&dungeon);
  }
  
  return 0;
}

void readDungeonInfo(FILE *dungeonInfo, Dungeon *dungeon){
    //Finds total bytes in document for use later
  char *buffer;
  buffer = malloc(4);
  fseek(dungeonInfo, 16, SEEK_SET);
  fread(buffer, 1, 4, dungeonInfo);
  int bytes;
  sscanf(buffer, "%d", &bytes);
  
  //Starts at byte 20 and takes all bytes which denote hardness
  unsigned char *buffer2;
  buffer2 = malloc(16800);
  fread(buffer2, 1, 16800, dungeonInfo);
  int i;
  for(i = 0; i < 16800; i++){
    dungeon->spaces[i/160][i%160].hardness = buffer2[i];
  }

  //Traverses through document to find room information
  unsigned char *buffer3;
  buffer3 = malloc(bytes-16800);
  Room rooms[(bytes-16800)/4];
  fread(buffer3, 1, bytes-16800, dungeonInfo);
  int j;
  int k = 0;
  for(j = 0; j < (bytes-16800)/4; j++){
    rooms[j].upperX = buffer3[k];
    rooms[j].upperY = buffer3[k+1];
    rooms[j].width = buffer3[k+2];
    rooms[j].height = buffer3[k+3];
    k+=4;
  }

    //If hardness is zero and not a room, changes to corridor. Otherwise, sets material to rock
  int o, p;
  for(o = 0; o < 105; o++){
    for(p = 0; p < 160; p++){
      if(dungeon->spaces[o][p].hardness == 0)
	dungeon->spaces[o][p].material = '#';
      else
	dungeon->spaces[o][p].material = ' ';
    }
  }

  //Plots rooms in dungeon based on info read from file
  int l;
  for(l = 0; l < (bytes-16800)/4; l++){
    int m, n;
    for(m = rooms[l].upperY; m < rooms[l].upperY + rooms[l].height; m++){
      for(n = rooms[l].upperX; n < rooms[l].upperX + rooms[l].width; n++){
	dungeon->spaces[m][n].material = '.';
      }
    }
  }

}
void writeDungeonInfo(FILE *dungeonInfo, Dungeon *dungeon){
  unsigned char* hardness;
  hardness = malloc(16800);
  int *rooms;
  rooms = malloc(100);
  int y = 0;
  int i, j;
  int k = 0;
  int topLeft = 0;
  //Records the hardness and statistics for all rooms
  for(i = 0; i < 105; i++){
    for(j = 0; j < 160; j++){
      hardness[k] = dungeon->spaces[i][j].hardness;
      k++;
      if(dungeon->spaces[i][j].material == '.' && dungeon->spaces[i-1][j].material == ' ' && dungeon->spaces[i-1][j-1].material == ' ' && dungeon->spaces[i][j-1].material == ' '){
	findRoom(&rooms, y, i, j, &dungeon);
        y++;
      }
    }
  }
  //Writes semantic marker to file in bytes 0-11
  fseek(dungeonInfo, 0, SEEK_SET);
  char *marker = "RLG327-S2017";
  fwrite(marker, 1, 12, dungeonInfo);

  //Writes version number 0 to bytes 12-15
  int version = 0;
  fwrite(version, 1, 4, dungeonInfo);

  //Writes size of file to bytes 16-19
  int size = 16800 + sizeof(rooms);
  fwrite(size, 1, 4, dungeonInfo);
  
  //Writes hardness of each space in bytes 20-16819
  fwrite(hardness, 1, 16800, dungeonInfo);
  
  //Write positions of rooms from byte 16820 until the last byte so that all rooms are written
  fwrite(rooms, 1, size-16800, dungeonInfo);
}

void findRoom(int *rooms, int index, int y, int x, Dungeon *dungeon){
  int ycpy = y;
  int xcpy = x;
  int width = 0;
  int height = 0;
  while(dungeon->spaces[ycpy][xcpy].material == '.'){
    width++;
    xcpy++;
  }
  ycpy = y;
  xcpy = x;
  while(dungeon->spaces[ycpy][xcpy].material == '.'){
    height++;
    ycpy++;
  }
  rooms[index] = x;
  rooms[index+1] = y;
  rooms[index+2] = width;
  rooms[index+3] = height;
}
