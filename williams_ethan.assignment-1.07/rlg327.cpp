#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
using namespace std;

#include "dungeon.h"
#include "pc.h"
#include "npc.h"
#include "move.h"
#include "io.h"

typedef struct monster_description{
  string NAME;
  string SYMB;
  string COLOR;
  string DESC;
  string SPEED;
  string DAM;
  string HP;
  string ABIL;
}monster_description_t;

const char *victory =
  "                                       o\n"
  "                                      $\"\"$o\n"
  "                                     $\"  $$\n"
  "                                      $$$$\n"
  "                                      o \"$o\n"
  "                                     o\"  \"$\n"
  "                oo\"$$$\"  oo$\"$ooo   o$    \"$    ooo\"$oo  $$$\"o\n"
  "   o o o o    oo\"  o\"      \"o    $$o$\"     o o$\"\"  o$      \"$  "
  "\"oo   o o o o\n"
  "   \"$o   \"\"$$$\"   $$         $      \"   o   \"\"    o\"         $"
  "   \"o$$\"    o$$\n"
  "     \"\"o       o  $          $\"       $$$$$       o          $  ooo"
  "     o\"\"\n"
  "        \"o   $$$$o $o       o$        $$$$$\"       $o        \" $$$$"
  "   o\"\n"
  "         \"\"o $$$$o  oo o  o$\"         $$$$$\"        \"o o o o\"  "
  "\"$$$  $\n"
  "           \"\" \"$\"     \"\"\"\"\"            \"\"$\"            \""
  "\"\"      \"\"\" \"\n"
  "            \"oooooooooooooooooooooooooooooooooooooooooooooooooooooo$\n"
  "             \"$$$$\"$$$$\" $$$$$$$\"$$$$$$ \" \"$$$$$\"$$$$$$\"  $$$\""
  "\"$$$$\n"
  "              $$$oo$$$$   $$$$$$o$$$$$$o\" $$$$$$$$$$$$$$ o$$$$o$$$\"\n"
  "              $\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\""
  "\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"$\n"
  "              $\"                                                 \"$\n"
  "              $\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\"$\""
  "$\"$\"$\"$\"$\"$\"$\"$\n"
  "                                   You win!\n";

const char *tombstone =
  "\n\n\n                /\"\"\"\"\"/\"\"\"\"\"\"\".\n"
  "               /     /         \\             __\n"
  "              /     /           \\            ||\n"
  "             /____ /   Rest in   \\           ||\n"
  "            |     |    Pieces     |          ||\n"
  "            |     |               |          ||\n"
  "            |     |   A. Luser    |          ||\n"
  "            |     |               |          ||\n"
  "            |     |     * *   * * |         _||_\n"
  "            |     |     *\\/* *\\/* |        | TT |\n"
  "            |     |     *_\\_  /   ...\"\"\"\"\"\"| |"
  "| |.\"\"....\"\"\"\"\"\"\"\".\"\"\n"
  "            |     |         \\/..\"\"\"\"\"...\"\"\""
  "\\ || /.\"\"\".......\"\"\"\"...\n"
  "            |     |....\"\"\"\"\"\"\"........\"\"\"\"\""
  "\"^^^^\".......\"\"\"\"\"\"\"\"..\"\n"
  "            |......\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"......"
  "..\"\"\"\"\"....\"\"\"\"\"..\"\"...\"\"\".\n\n"
  "            You're dead.  Better luck in the next life.\n\n";

void usage(char *name)
{
  fprintf(stderr,
          "Usage: %s [-r|--rand <seed>] [-l|--load [<file>]]\n"
          "          [-s|--save [<file>]] [-i|--image <pgm file>]\n"
          "          [-p|--pc <y> <x>] [-n|--nummon <count>]\n",
          name);

  exit(-1);
}

int main(int argc, char *argv[])
{
  //Checks to make sure path exists
  char *dir = getenv("HOME");
  char *path = strcat(dir, "/.rlg327/");
  mkdir(path, 0777);
  
  //Opens monster description text file
  std::ifstream monster_desc;
  monster_desc.open(strcat(path, "/monster_desc.txt"));
  string metadata;
  getline(monster_desc, metadata);
  //If metadata doesn't match, exit program
  if(metadata != "RLG327 MONSTER DESCRIPTION 1"){
    return 0;
  }
  monster_description_t tmp_monster;
  string tmp;
  int flag = 0;
  while(getline(monster_desc, tmp)){
    if(tmp == "" || tmp == "BEGIN MONSTER"){
      continue;
    }
    if(tmp ==  "END"){
      if(tmp_monster.NAME == "" || tmp_monster.SYMB == "" || tmp_monster.COLOR == "" ||
	 tmp_monster.SPEED == "" || tmp_monster.DAM == "" || tmp_monster.HP == "" ||
	 tmp_monster.ABIL == "" || tmp_monster.DESC == "" || flag == 1){
        tmp_monster.NAME = tmp_monster.SYMB = tmp_monster.COLOR = tmp_monster.SPEED = tmp_monster.DAM = tmp_monster.HP = tmp_monster.ABIL = tmp_monster.DESC = "";
	flag = 0;
      }
      else{
	//monster has everything, print to console
	cout << "Name: " << tmp_monster.NAME << '\n';
	cout << "Symbol: " << tmp_monster.SYMB << '\n';
	cout << "Color: " << tmp_monster.COLOR << '\n';
	cout << "Speed: " << tmp_monster.SPEED << '\n';
	cout << "Damage: " << tmp_monster.DAM << '\n';
	cout << "Hit points: " << tmp_monster.HP << '\n';
	cout << "Abillities: " << tmp_monster.ABIL << '\n';
	cout << "Description: " << tmp_monster.DESC << '\n';;
	cout << '\n' << '\n';
	tmp_monster.NAME = tmp_monster.SYMB = tmp_monster.COLOR = tmp_monster.SPEED = tmp_monster.DAM = tmp_monster.HP = tmp_monster.ABIL = tmp_monster.DESC = ""; 
      }
    }
    else if(tmp.substr(0, 4) == "NAME"){
      if(tmp_monster.NAME != ""){
	flag = 1;
      }
      tmp_monster.NAME = tmp.substr(5, tmp.length());
    }
    else if(tmp.substr(0, 4) == "SYMB"){
      if(tmp_monster.SYMB != ""){
	flag = 1;
      }
      tmp_monster.SYMB = tmp.substr(5, tmp.length());
    }
    else if(tmp.substr(0, 5) == "COLOR"){
      if(tmp_monster.COLOR != ""){
	flag = 1;
      }
      tmp_monster.COLOR = tmp.substr(5, tmp.length());
    }
    else if(tmp.substr(0, 5) == "SPEED"){
      if(tmp_monster.SPEED != ""){
	flag = 1;
      }
      tmp_monster.SPEED = tmp.substr(6, tmp.length());
    }
    else if(tmp.substr(0, 3) == "DAM"){
      if(tmp_monster.DAM != ""){
	flag = 1;
      }
      tmp_monster.DAM = tmp.substr(4, tmp.length());
    }
    else if(tmp.substr(0, 2) == "HP"){
      if(tmp_monster.HP != ""){
	flag = 1;
      }
      tmp_monster.HP = tmp.substr(3, tmp.length());
    }
    else if(tmp.substr(0, 4) == "ABIL"){
      if(tmp_monster.ABIL != ""){
	flag = 1;
      }
      tmp_monster.ABIL = tmp.substr(5, tmp.length());
    }
    else if(tmp.substr(0, 4) == "DESC"){
      if(tmp_monster.DESC != ""){
	flag = 1;
      }
      while(getline(monster_desc, tmp)){
	if(tmp == "."){
	  break;
	}
	if(tmp == "DESC"){
	  continue;
	}
	else if(tmp_monster.DESC == ""){
	  if(tmp.length() > 77){
	    flag = 1;
	  }
	  tmp_monster.DESC = tmp + '\n';
	}
	else{
	  if(tmp.length() > 77){
	    flag = 1;
	  }
	  tmp_monster.DESC = tmp_monster.DESC + tmp + '\n';
	}
      }
    }
    else{
      flag = 1;
    }
  }
  monster_desc.close();
  
  return 0;
  
  dungeon_t d;
  time_t seed;
  struct timeval tv;
  int32_t i;
  uint32_t do_load, do_save, do_seed, do_image, do_place_pc;
  uint32_t long_arg;
  char *save_file;
  char *load_file;
  char *pgm_file;

  memset(&d, 0, sizeof (d));

  /* Default behavior: Seed with the time, generate a new dungeon, *
   * and don't write to disk.                                      */
  do_load = do_save = do_image = do_place_pc = 0;
  do_seed = 1;
  save_file = load_file = NULL;
  d.max_monsters = MAX_MONSTERS;

  /* The project spec requires '--load' and '--save'.  It's common  *
   * to have short and long forms of most switches (assuming you    *
   * don't run out of letters).  For now, we've got plenty.  Long   *
   * forms use whole words and take two dashes.  Short forms use an *
    * abbreviation after a single dash.  We'll add '--rand' (to     *
   * specify a random seed), which will take an argument of it's    *
   * own, and we'll add short forms for all three commands, '-l',   *
   * '-s', and '-r', respectively.  We're also going to allow an    *
   * optional argument to load to allow us to load non-default save *
   * files.  No means to save to non-default locations, however.    *
   * And the final switch, '--image', allows me to create a dungeon *
   * from a PGM image, so that I was able to create those more      *
   * interesting test dungeons for you.                             */
  
 if (argc > 1) {
    for (i = 1, long_arg = 0; i < argc; i++, long_arg = 0) {
      if (argv[i][0] == '-') { /* All switches start with a dash */
        if (argv[i][1] == '-') {
          argv[i]++;    /* Make the argument have a single dash so we can */
          long_arg = 1; /* handle long and short args at the same place.  */
        }
        switch (argv[i][1]) {
        case 'r':
          if ((!long_arg && argv[i][2]) ||
              (long_arg && strcmp(argv[i], "-rand")) ||
              argc < ++i + 1 /* No more arguments */ ||
              !sscanf(argv[i], "%lu", &seed) /* Argument is not an integer */) {
            usage(argv[0]);
          }
          do_seed = 0;
          break;
        case 'l':
          if ((!long_arg && argv[i][2]) ||
              (long_arg && strcmp(argv[i], "-load"))) {
            usage(argv[0]);
          }
          do_load = 1;
          if ((argc > i + 1) && argv[i + 1][0] != '-') {
            /* There is another argument, and it's not a switch, so *
             * we'll treat it as a save file and try to load it.    */
            load_file = argv[++i];
          }
          break;
        case 's':
          if ((!long_arg && argv[i][2]) ||
              (long_arg && strcmp(argv[i], "-save"))) {
            usage(argv[0]);
          }
          do_save = 1;
          if ((argc > i + 1) && argv[i + 1][0] != '-') {
            /* There is another argument, and it's not a switch, so *
             * we'll treat it as a save file and try to load it.    */
            save_file = argv[++i];
          }
          break;
        case 'i':
          if ((!long_arg && argv[i][2]) ||
              (long_arg && strcmp(argv[i], "-image"))) {
            usage(argv[0]);
          }
          do_image = 1;
          if ((argc > i + 1) && argv[i + 1][0] != '-') {
            /* There is another argument, and it's not a switch, so *
             * we'll treat it as a save file and try to load it.    */
            pgm_file = argv[++i];
          }
          break;
        case 'n':
          if ((!long_arg && argv[i][2]) ||
              (long_arg && strcmp(argv[i], "-nummon")) ||
              argc < ++i + 1 /* No more arguments */ ||
              !sscanf(argv[i], "%hu", &d.max_monsters)) {
            usage(argv[0]);
          }
          break;
          /*
        case 'p':
          if ((!long_arg && argv[i][2])            ||
              (long_arg && strcmp(argv[i], "-pc")) ||
              argc <= i + 2                        ||
              argv[i + 1][0] == '-'                ||
              argv[i + 2][0] == '-') {
            usage(argv[0]);
          }
          do_place_pc = 1;
          if (character_set_y(d.PC, atoi(argv[++i])) < 1 ||
              character_get_y(d.PC) > DUNGEON_Y - 2         ||
              character_set_x(d.PC, atoi(argv[++i])) < 1 ||
              character_get_x(d.PC) > DUNGEON_X - 2) {
            fprintf(stderr, "Invalid PC position.\n");
            usage(argv[0]);
          }
          break;
          */
        default:
          usage(argv[0]);
        }
      } else { /* No dash */
        usage(argv[0]);
      }
    }
  }

  if (do_seed) {
    /* Allows me to generate more than one dungeon *
     * per second, as opposed to time().           */
    gettimeofday(&tv, NULL);
    seed = (tv.tv_usec ^ (tv.tv_sec << 20)) & 0xffffffff;
  }

  printf("Seed is %ld.\n", seed);
  srand(seed);

  io_init_terminal();
  init_dungeon(&d);

  if (do_load) {
    read_dungeon(&d, load_file);
  } else if (do_image) {
    read_pgm(&d, pgm_file);
  } else {
    gen_dungeon(&d);
  }

  config_pc(&d);
  gen_monsters(&d);

  io_display(&d);
  while (pc_is_alive(&d) && dungeon_has_npcs(&d) && !d.quit) {
    do_moves(&d);
  }
  io_display(&d);

  if (!d.quit) {
    sleep(2);
  }

  io_reset_terminal();

  if (do_save) {
    write_dungeon(&d, save_file);
  }

  if(pc_is_alive(&d)){
    printf("%s", victory);
  }
  else{
    printf("%s", tombstone);
  }
  printf("\nYou defended your life in the face of %u deadly beast%s.\n"
         "You avenged the cruel and untimely murders of %u "
         "peaceful dungeon resident%s.\n",
         character_get_dkills(d.PC),
         character_get_dkills(d.PC) == 1 ? "" : "s",
         character_get_ikills(d.PC),
         character_get_ikills(d.PC) == 1 ? "" : "s");

  if (pc_is_alive(&d)) {
    /* If the PC is dead, it's in the move heap and will get automatically *
     * deleted when the heap destructs.  In that case, we can't call       *
     * delete_pc(), because it will lead to a double delete.               */
    character_delete(d.PC);
  }

  delete_dungeon(&d);

  return 0;
}
