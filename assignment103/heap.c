#include <stdio.h>
#include "heap.h"

void extractMin(Heap* heap, Node* root){
  root = heap->min;
  Node* newMin = heap->min->next;
  heap->min = newMin;
  root->next = NULL;
  heap->min->previous = NULL;
  heap->size--;
}

void addNode(Heap* heap, Space* dungeonSpace, int sourceDis){
  //Goes through nodes starting at minimum and compares distances from source node
  Node adding;
  adding.space = dungeonSpace;
  adding.srcDis = sourceDis;
  if(heap->size == 0){
    adding.previous = NULL;
    adding.next = NULL;
    heap->min = &adding;
    printf("First node placed: %d\n", heap->min->srcDis);
  }
  else{
    int placed = 0;
    int num = 0;
    Node* cur = heap->min;
    while(placed == 0){
      if(!cur->next){
	adding.previous = cur;
	adding.next = NULL;
	cur->next = &adding;
	placed = 1;
      }
      else if(cur->srcDis <= sourceDis)
	cur = cur->next;
      else{
	adding.next = cur;
	adding.previous = cur->previous;
	adding.previous->next = &adding;
	cur->previous = &adding;
	placed = 1;
      }
    }
  }
}
