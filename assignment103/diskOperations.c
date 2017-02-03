#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include "generateDungeon.h"
#include "diskOperations.h"
#include <endian.h>

void saveload(Dungeon *dungeon, Rooms *rooms, char *arg1, char *arg2){

  //Checks if desired directory exists, creates directory if not
  char *dir = getenv("HOME");
  char *path = strcat(dir, "/.rlg327/");
  mkdir(path, 0777);
  
  if(arg1 == NULL){
    generateRooms(dungeon);
    
    printDungeon(dungeon);
  }
  //If user adds save switch only
  else if(strcmp(arg1, "--SAVE") == 0 && arg2 == NULL){
    FILE *dungeonInfo = fopen(strcat(path, "dungeon"), "w");

    generateRooms(dungeon);
    
    writeDungeonInfo(dungeonInfo, dungeon, rooms);

    fclose(dungeonInfo);
  }
  //Is user adds both save and load switches
  else if(strcmp(arg1, "--SAVE") == 0 && strcmp(arg2, "--LOAD") == 0){
    //Reads dungeon from disc and displays it
    FILE *dungeonInfo = fopen(strcat(path, "dungeon"), "r");
    
    readDungeonInfo(dungeonInfo, dungeon, rooms);

    printDungeon(dungeon);

    fclose(dungeonInfo);

    //Rewrites dungeon to disk
    FILE *dungeonInfo2 = fopen(strcat(path, "dungeon"), "w");
    
    writeDungeonInfo(dungeonInfo2, dungeon, rooms);

    fclose(dungeonInfo2);
  }
  //If user adds load switch with specific file
  else if(strcmp(arg1, "--LOAD") == 0 && arg2 != NULL){
    FILE *dungeonInfo = fopen(strcat(path, arg2), "r");

    readDungeonInfo(dungeonInfo, dungeon, rooms);

    printDungeon(dungeon);

    fclose(dungeonInfo);
  }
  //If user adds load switch with no file it wil default to loading dungeon
  else if(strcmp(arg1, "--LOAD") == 0){
    FILE *dungeonInfo = fopen(strcat(path, "dungeon"), "r");

    readDungeonInfo(dungeonInfo, dungeon, rooms);

    printDungeon(dungeon);

    fclose(dungeonInfo);
  }
  //If user adds an unrecognized switch/input, displays random dungeon and exits
  else{
    generateRooms(dungeon);
    
    printDungeon(dungeon);
  }
}

void readDungeonInfo(FILE *dungeonInfo, Dungeon *dungeon, Rooms *rooms){
  //Finds total bytes in document for use later
  int bytes;
  fseek(dungeonInfo, 16, SEEK_SET);
  fread(&bytes, sizeof(bytes), 1, dungeonInfo);
  bytes = htobe32(bytes);
  printf("%d\n", bytes);
  
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
  Room *roomArr;
  roomArr = malloc((bytes-16800)/4);
  fread(buffer3, 1, bytes-16800, dungeonInfo);
  int j;
  int k = 0;
  for(j = 0; j < (bytes-16820)/4; j++){
    roomArr[j].upperX = buffer3[k];
    roomArr[j].upperY = buffer3[k+1];
    roomArr[j].width = buffer3[k+2];
    roomArr[j].height = buffer3[k+3];
    k+=4;
  }

  rooms->allRooms = roomArr;

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
    for(m = roomArr[l].upperY; m < roomArr[l].upperY + roomArr[l].height; m++){
      for(n = roomArr[l].upperX; n < roomArr[l].upperX + roomArr[l].width; n++){
	dungeon->spaces[m][n].material = '.';
      }
    }
  }
}
void writeDungeonInfo(FILE *dungeonInfo, Dungeon *dungeon, Rooms *rooms){
  unsigned char *hard;
  hard = malloc(16800);
  int s, t;
  int k = 0;
  
  //Records the hardness and statistics for all rooms
  for(s = 0; s < 105; s++){
    for(t = 0; t < 160; t++){
      hard[k] =  (unsigned char) dungeon->spaces[s][t].hardness;
      k++;
    }
  }

  //Creates array of room statistics for placement in bytes 18620-end
  Room *roomStats;
  roomStats = malloc(18);
  int u, v;
  int y = 0;
  for(u = 0; u < 105; u++){
    for(v = 0; v < 160; v++){
      if(dungeon->spaces[u][v].material == '.' && dungeon->spaces[u-1][v].material != '.' && dungeon->spaces[u-1][v-1].material != '.' && dungeon->spaces[u][v-1].material != '.'){
          int ycpy = u;
	  int xcpy = v;
	  int width = 0;
	  int height = 0;
	  while(dungeon->spaces[ycpy][xcpy].material == '.'){
	    width++;
	    xcpy++;
	  }
	  int ycpy2 = u;
	  int xcpy2 = v;
	  while(dungeon->spaces[ycpy2][xcpy2].material == '.'){
	    height++;
	    ycpy2++;
	  }
	  roomStats[y].upperY = v; 
	  roomStats[y].upperX = u;
	  roomStats[y].width = width;
	  roomStats[y].height = height;
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
  version = htobe32(version);
  fwrite(&version, sizeof(version), 1, dungeonInfo);
  
  //Writes size of file to bytes 16-19
  int size = 16892;
  size = htobe32(size);
  fwrite(&size, sizeof(size), 1, dungeonInfo);
  
  //Writes hardness of each space in bytes 20-16819
  fwrite(hard, 1, 16800, dungeonInfo);
  
  //Write positions of rooms from byte 16820 until the last byte so that all rooms are written
  unsigned char *roomInfo;
  roomInfo = malloc(72);
  int q;
  int yz = 0;
  for(q = 0; q < 72; q+=4){
    roomInfo[q] = roomStats[yz].upperY;
    roomInfo[q+1] = roomStats[yz].upperX;
    roomInfo[q+2] = roomStats[yz].width;
    roomInfo[q+3] = roomStats[yz].height;
    yz++;
   }
  fwrite(roomInfo, 1, size-16800, dungeonInfo);
}
