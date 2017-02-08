#ifndef HEAP_H
#define HEAP_H

#include "generateDungeon.h"

typedef struct Node Node;

struct Node{
  Node* previous;
  Node* next;
  Space* space;
  int srcDis;
};

typedef struct{
  Node* min;
  int size;
}Heap;

void extractMin(Heap* heap, Node* root);
  
void addNode(Heap* heap, Space* dungeonSpace, int sourceDis);

#endif
