#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "generateDungeon.h"
#include "diskOperations.h"
#include "heap.h"

int main(int argc, char *argv[]){
  Dungeon dungeon;
  
  typedef struct{
  int y;
  int x;
  }Location;

  saveload(&dungeon, argv[1], argv[2]);

  Location playerLoc;

  srand(time(NULL));
  int placed = 0;
  while(!placed){
    int y = rand() % 105;
    int x = rand() % 160;
    if(dungeon.spaces[y][x].material == '.'){
      dungeon.spaces[y][x].material = '@';
      playerLoc.y = y;
      playerLoc.x = x;
      placed = 1;
    }
  }
  
  //Calculate distances for non-tunneling monsters
  
  //If hardness is zero and the space doesn't contain the player add it to the heap
  Heap heap;
  heap.min = NULL;
  heap.size = 0;
  int i, j;
  int num = 0;
  for(i = 0; i < 105; i++){
    for(j = 0; j < 105; j++){
      if(dungeon.spaces[i][j].hardness == 0 && i != playerLoc.y && j != playerLoc.x && num < 10){
	addNode(&heap, &dungeon.spaces[i][j], 105-i + 160-j);
	dungeon.spaces[i][j].nonTunnelDistance = 1000000;
	heap.size++;
	num++;
      }
      else
	dungeon.spaces[i][j].nonTunnelDistance = 500000; 
    }
  }

  Node *cur = heap.min;
  while(cur->next != NULL){
    printf("%d\n", cur->srcDis);
  }
  
  //Go through heap and checks neighbor of minimum priority node, finding least distance to it.
  while(heap.size > 0){
    Node u; 
    extractMin(&heap, &u);
    //If the current node's distance from source plus the distance between the node and its (up and left) neighbor is less than the current distance from the source to the neighbor, update neighbors's distance
    if(u.space->nonTunnelDistance + 1 < dungeon.spaces[u.space->y-1][u.space->x-1].nonTunnelDistance){
      dungeon.spaces[u.space->y-1][u.space->x-1].nonTunnelDistance = u.space->nonTunnelDistance + 1;
    }
    //If the current node's distance from source plus the distance between the node and its (up) neighbor is less than the current distance from the source to the neighbor, update neighbors's distance
    if(u.space->nonTunnelDistance + 1 < dungeon.spaces[u.space->y-1][u.space->x].nonTunnelDistance){
      dungeon.spaces[u.space->y-1][u.space->x].nonTunnelDistance = u.space->nonTunnelDistance + 1;
    }
        //If the current node's distance from source plus the distance between the node and its (up and right) neighbor is less than the current distance from the source to the neighbor, update neighbors's distance
    if(u.space->nonTunnelDistance + 1 < dungeon.spaces[u.space->y-1][u.space->x+1].nonTunnelDistance){
      dungeon.spaces[u.space->y-1][u.space->x+1].nonTunnelDistance = u.space->nonTunnelDistance + 1;
    }
        //If the current node's distance from source plus the distance between the node and its (right) neighbor is less than the current distance from the source to the neighbor, update neighbors's distance
    if(u.space->nonTunnelDistance + 1 < dungeon.spaces[u.space->y][u.space->x+1].nonTunnelDistance){
      dungeon.spaces[u.space->y][u.space->x+1].nonTunnelDistance = u.space->nonTunnelDistance + 1;
    }
        //If the current node's distance from source plus the distance between the node and its (down and right) neighbor is less than the current distance from the source to the neighbor, update neighbors's distance
    if(u.space->nonTunnelDistance + 1 < dungeon.spaces[u.space->y+1][u.space->x+1].nonTunnelDistance){
      dungeon.spaces[u.space->y+1][u.space->x+1].nonTunnelDistance = u.space->nonTunnelDistance + 1;
    }
        //If the current node's distance from source plus the distance between the node and its (down) neighbor is less than the current distance from the source to the neighbor, update neighbors's distance
    if(u.space->nonTunnelDistance + 1 < dungeon.spaces[u.space->y+1][u.space->x].nonTunnelDistance){
      dungeon.spaces[u.space->y+1][u.space->x].nonTunnelDistance = u.space->nonTunnelDistance + 1;
    }
        //If the current node's distance from source plus the distance between the node and its (down and left) neighbor is less than the current distance from the source to the neighbor, update neighbors's distance
    if(u.space->nonTunnelDistance + 1 < dungeon.spaces[u.space->y+1][u.space->x-1].nonTunnelDistance){
      dungeon.spaces[u.space->y+1][u.space->x-1].nonTunnelDistance = u.space->nonTunnelDistance + 1;
    }
        //If the current node's distance from source plus the distance between the node and its (left) neighbor is less than the current distance from the source to the neighbor, update neighbors's distance
    if(u.space->nonTunnelDistance + 1 < dungeon.spaces[u.space->y][u.space->x-1].nonTunnelDistance){
      dungeon.spaces[u.space->y][u.space->x-1].nonTunnelDistance = u.space->nonTunnelDistance + 1;
    }
  }

  int u, v;
  for(u = 0; u < 105; u++){
    for(v = 0; v < 160; v++){
      if(dungeon.spaces[u][v].nonTunnelDistance != 500000){
	printf("%c", dungeon.spaces[u][v].nonTunnelDistance % 10);

      }
      else{
	printf("%c", ' ');
      }
    }
    printf("\n");
  }
  
  //Calculate distances for tunneling monsters

  
  return 0;
}
