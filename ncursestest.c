#include <stdio.h>
#include <ncurses.h>

int main(int argc, char *argv[]){
  initscr();
  noecho();
  while(1){
    char c = getch();
    addch(c);
    refresh();
  }
  endwin();

  return 0;
}
