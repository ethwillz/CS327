#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

char board[24][80], neighbors[24][80];
int alive = 0;

int numNeighbors(int x, int y);
void printBoard();

int main(int argc, char *argv[]){
 
    //Initialize game board with cells
    int i;
    int j = 0;
    for(i = 1; i < argc; i+=2){
      int x = atoi(argv[i+1]);
      int y = atoi(argv[i]);
      board[x][y] = 'O';
      j++;
      alive++;
    }

    //Designates the board's next state based on current position of organisms
    int x, y;
      while(alive > 0){
		//Populates the map for the number of neighbors and fills empty positions with spaces (only applicable on first run)
		for(x = 0; x < 24; x++){
		  for(y = 0; y < 80; y++){
			neighbors[x][y] = numNeighbors(x, y);
			if(board[x][y] != 'O'){board[x][y] = ' ';}
		  }
		}

		printBoard();
		
		for(x = 0; x < 24; x++){
		  for(y = 0; y < 80; y++){
			//First rule, 0 or 1 live  neighbors
			if((neighbors[x][y] < 2 && board[x][y]=='O')){
			  board[x][y]=' ';
			  alive--;
			}
			//Third rule, 4 live neighbors
			if(neighbors[x][y] > 3 && board[x][y] == 'O'){
			  board[x][y]=' ';
			  alive--;
			}
			//Rule 4, dead and 3 live neighbors
			if(neighbors[x][y] == 3 && board[x][y] == ' '){
			  board[x][y]='O';
			  alive++;
			}
		  }
		}

		usleep(83333);
      }
    return 0;
}

//Finds number of neighbors of a given cell
int numNeighbors(int x, int y){
  int num = 0;
  int i;
  for(i = x - 1; i < x+2; i++){
    if(board[(i+24)%24][(y+80)%80]=='O' && i != x){num++;}
    if(board[(i+24)%24][((y-1+80)%80)]=='O'){num++;}
    if(board[(i+24)%24][(y+1+80)%80]=='O'){num++;}
  }
  return num;
}

//Prints entire board
void printBoard(){
  int i, j;
  for(i = 0; i < 24; i++){
    for(j = 0; j < 80; j++){
      printf("%c", board[i][j]);
    }
    printf("\n");
  }
}
