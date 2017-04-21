#include <unistd.h>
#include <ncurses.h>
#include <ctype.h>
#include <stdlib.h>

#include "io.h"
#include "move.h"
#include "path.h"
#include "pc.h"
#include "utils.h"
#include "dungeon.h"
#include "object.h"

/* Same ugly hack we did in path.c */
static dungeon_t *dungeon;

typedef struct io_message {
  /* Will print " --more-- " at end of line when another message follows. *
   * Leave 10 extra spaces for that.                                      */
  char msg[71];
  struct io_message *next;
} io_message_t;

static io_message_t *io_head, *io_tail;

void io_init_terminal(void)
{
  initscr();
  raw();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);
  start_color();
  init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
  init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
  init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
  init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
  init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
  init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
    init_color(10, 66, 179, 244);
    init_color(11, 244, 131, 66);
}

void io_reset_terminal(void)
{
  endwin();

  while (io_head) {
    io_tail = io_head;
    io_head = io_head->next;
    free(io_tail);
  }
  io_tail = NULL;
}

void io_queue_message(const char *format, ...)
{
  io_message_t *tmp;
  va_list ap;

  if (!(tmp = (io_message_t *) malloc(sizeof (*tmp)))) {
    perror("malloc");
    exit(1);
  }

  tmp->next = NULL;

  va_start(ap, format);

  vsnprintf(tmp->msg, sizeof (tmp->msg), format, ap);

  va_end(ap);

  if (!io_head) {
    io_head = io_tail = tmp;
  } else {
    io_tail->next = tmp;
    io_tail = tmp;
  }
}

static void io_print_message_queue(uint32_t y, uint32_t x)
{
  while (io_head) {
    io_tail = io_head;
    attron(COLOR_PAIR(COLOR_CYAN));
    mvprintw(y, x, "%-80s", io_head->msg);
    attroff(COLOR_PAIR(COLOR_CYAN));
    io_head = io_head->next;
    if (io_head) {
      attron(COLOR_PAIR(COLOR_CYAN));
      mvprintw(y, x + 70, "%10s", " --more-- ");
      attroff(COLOR_PAIR(COLOR_CYAN));
      refresh();
      getch();
    }
    free(io_tail);
  }
  io_tail = NULL;
}

/* Possible offsets:
 *   ( 0,  0), (20,  0), (40,  0), (60,  0), (80,  0) *
 *   ( 0,  7), (20,  7), (40,  7), (60,  7), (80,  7) *
 *   ( 0, 14), (20, 14), (40, 14), (60, 14), (80, 14) *
 *   ( 0, 21), (20, 21), (40, 21), (60, 21), (80, 21) *
 *   ( 0, 28), (20, 28), (40, 28), (60, 28), (80, 28) *
 *   ( 0, 35), (20, 35), (40, 35), (60, 35), (80, 35) *
 *   ( 0, 42), (20, 42), (40, 42), (60, 42), (80, 42) *
 *   ( 0, 49), (20, 49), (40, 49), (60, 49), (80, 49) *
 *   ( 0, 56), (20, 56), (40, 56), (60, 56), (80, 56) *
 *   ( 0, 63), (20, 63), (40, 63), (60, 63), (80, 63) *
 *   ( 0, 70), (20, 70), (40, 70), (60, 70), (80, 70) *
 *   ( 0, 77), (20, 77), (40, 77), (60, 77), (80, 77) *
 *   ( 0, 84), (20, 84), (40, 84), (60, 84), (80, 84) */

void io_calculate_offset(dungeon_t *d)
{
  d->io_offset[dim_x] = ((character_get_x(d->PC) - 10) / 20) * 20;
  if (d->io_offset[dim_x] < 0) {
    d->io_offset[dim_x] = 0;
  }
  if (d->io_offset[dim_x] > 80) {
    d->io_offset[dim_x] = 80;
  }
  d->io_offset[dim_y] = ((character_get_y(d->PC) - 4) / 7) * 7;
  if (d->io_offset[dim_y] < 0) {
    d->io_offset[dim_y] = 0;
  }
  if (d->io_offset[dim_y] > 84) {
    d->io_offset[dim_y] = 84;
  }
}

void io_update_offset(dungeon_t *d)
{
  int32_t x, y;

  x = (40 + d->io_offset[dim_x]) - character_get_x(d->PC);
  y = (11 + d->io_offset[dim_y]) - character_get_y(d->PC);

  if (x >= 15 && d->io_offset[dim_x]) {
    d->io_offset[dim_x] -= 20;
  }
  if (x <= -15 && d->io_offset[dim_x] != 80) {
    d->io_offset[dim_x] += 20;
  }
  if (y >= 5 && d->io_offset[dim_y]) {
    d->io_offset[dim_y] -= 7;
  }
  if (y <= -5 && d->io_offset[dim_y] != 84) {
    d->io_offset[dim_y] += 7;
  }
}

void io_display_tunnel(dungeon_t *d)
{
  uint32_t y, x;
  clear();
  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      mvaddch(y + 1, x, '0' + (d->pc_tunnel[y][x] % 10));
    }
  }
  refresh();
}

void io_display_distance(dungeon_t *d)
{
  uint32_t y, x;
  clear();
  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      mvaddch(y + 1, x, '0' + (d->pc_distance[y][x] % 10));
    }
  }
  refresh();
}

static char hardness_to_char[] =
  "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

void io_display_hardness(dungeon_t *d)
{
  uint32_t y, x;
  clear();
  for (y = 0; y < DUNGEON_Y; y++) {
    for (x = 0; x < DUNGEON_X; x++) {
      /* Maximum hardness is 255.  We have 62 values to display it, but *
       * we only want one zero value, so we need to cover [1,255] with  *
       * 61 values, which gives us a divisor of 254 / 61 = 4.164.       *
       * Generally, we want to avoid floating point math, but this is   *
       * not gameplay, so we'll make an exception here to get maximal   *
       * hardness display resolution.                                   */
      mvaddch(y + 1, x, (d->hardness[y][x]                             ?
                         hardness_to_char[1 + (d->hardness[y][x] / 5)] :
                         '0'));
    }
  }
  refresh();
}

static void io_redisplay_visible_monsters(dungeon_t *d)
{
  /* This was initially supposed to only redisplay visible monsters.  After *
   * implementing that (comparitivly simple) functionality and testing, I   *
   * discovered that it resulted to dead monsters being displayed beyond    *
   * their lifetimes.  So it became necessary to implement the function for *
   * everything in the light radius.  In hindsight, it would be better to   *
   * keep a static array of the things in the light radius, generated in    *
   * io_display() and referenced here to accelerate this.  The whole point  *
   * of this is to accelerate the rendering of multi-colored monsters, and  *
   * it is *significantly* faster than that (it eliminates flickering       *
   * artifacts), but it's still significantly slower than it could be.  I   *
   * will revisit this in the future to add the acceleration matrix.        */
  pair_t pc_offset;
  pair_t pos;
  uint32_t color;
  uint32_t illuminated;

  pc_offset[dim_y] = d->PC->position[dim_y] - d->io_offset[dim_y] + 1;
  pc_offset[dim_x] = d->PC->position[dim_x] - d->io_offset[dim_x];
  

  for (pos[dim_y] = -PC_VISUAL_RANGE;
       pos[dim_y] <= PC_VISUAL_RANGE;
       pos[dim_y]++) {
    for (pos[dim_x] = -PC_VISUAL_RANGE;
         pos[dim_x] <= PC_VISUAL_RANGE;
         pos[dim_x]++) {
      if ((d->io_offset[dim_y] + pc_offset[dim_y] + pos[dim_y] < 0) ||
          (d->io_offset[dim_y] + pc_offset[dim_y] + pos[dim_y] >= DUNGEON_Y) ||
          (d->io_offset[dim_x] + pc_offset[dim_x] + pos[dim_x] < 0) ||
          (d->io_offset[dim_x] + pc_offset[dim_x] + pos[dim_x] >= DUNGEON_X)) {
        continue;
      }
      if ((illuminated = is_illuminated(d->PC,
                                        d->io_offset[dim_y] +
                                        pc_offset[dim_y]    +
                                        pos[dim_y],
                                        d->io_offset[dim_x] +
                                        pc_offset[dim_x]    +
                                        pos[dim_x]))) {
        attron(A_BOLD);
      }
      if (d->character_map[d->io_offset[dim_y] +
                           pc_offset[dim_y]    +
                           pos[dim_y]]
                          [d->io_offset[dim_x] +
                           pc_offset[dim_x]    +
                           pos[dim_x]]           &&
          can_see(d,
                  d->PC->position,
                  d->character_map[d->io_offset[dim_y] +
                                   pc_offset[dim_y] +
                                   pos[dim_y]]
                                  [d->io_offset[dim_x] +
                                   pc_offset[dim_x] +
                                   pos[dim_x]]->position,
                  1, 0)) {
        attron(COLOR_PAIR((color =
                           d->character_map[d->io_offset[dim_y] +
                                            pc_offset[dim_y] +
                                            pos[dim_y]]
                                           [d->io_offset[dim_x] +
                                            pc_offset[dim_x] +
                                            pos[dim_x]]->get_color())));
        mvaddch(pc_offset[dim_y] + pos[dim_y] + 1,
                pc_offset[dim_x] + pos[dim_x],
                character_get_symbol(d->character_map[d->io_offset[dim_y] +
                                                      pc_offset[dim_y] +
                                                      pos[dim_y]]
                                                     [d->io_offset[dim_x] +
                                                      pc_offset[dim_x] +
                                                      pos[dim_x]]));
        attroff(COLOR_PAIR(color));
      } else if (d->objmap[d->io_offset[dim_y] +
                           pc_offset[dim_y]    +
                           pos[dim_y]]
                          [d->io_offset[dim_x] +
                           pc_offset[dim_x]    +
                           pos[dim_x]]                           &&
                 (can_see(d,
                          d->PC->position,
                          d->objmap[d->io_offset[dim_y] +
                                    pc_offset[dim_y] +
                                    pos[dim_y]]
                                   [d->io_offset[dim_x] +
                                    pc_offset[dim_x] +
                                    pos[dim_x]]->get_position(),
                          1, 0)                                  ||
                 d->objmap[d->io_offset[dim_y] +
                                    pc_offset[dim_y] +
                                    pos[dim_y]]
                                   [d->io_offset[dim_x] +
                                    pc_offset[dim_x] +
                                    pos[dim_x]]->have_seen())) {
        attron(COLOR_PAIR(d->objmap[d->io_offset[dim_y] +
                                    pc_offset[dim_y]    +
                                    pos[dim_y]]
                                   [d->io_offset[dim_x] +
                                    pc_offset[dim_x]    +
                                    pos[dim_x]]->get_color()));
        mvaddch(pc_offset[dim_y] + pos[dim_y] + 1,
                pc_offset[dim_x] + pos[dim_x],
                d->objmap[d->io_offset[dim_y] + pc_offset[dim_y] + pos[dim_y]]
                         [d->io_offset[dim_x] +
                          pc_offset[dim_x] +
                          pos[dim_x]]->get_symbol());
        attroff(COLOR_PAIR(d->objmap[d->io_offset[dim_y] +
                                     pc_offset[dim_y]    +
                                     pos[dim_y]]
                                    [d->io_offset[dim_x] +
                                     pc_offset[dim_x]    +
                                     pos[dim_x]]->get_color()));
      } else {
        switch (pc_learned_terrain(d->PC,
                                   d->io_offset[dim_y] +
                                   pc_offset[dim_y] + pos[dim_y],
                                   d->io_offset[dim_x] + pc_offset[dim_x] +
                                   pos[dim_x])) {
        case ter_wall:
        case ter_wall_immutable:
        case ter_unknown:
          mvaddch(pc_offset[dim_y] + pos[dim_y] + 1,
                  pc_offset[dim_x] + pos[dim_x], ' ');
          break;
        case ter_floor:
        case ter_floor_room:
          mvaddch(pc_offset[dim_y] + pos[dim_y] + 1,
                  pc_offset[dim_x] + pos[dim_x], '.');
          break;
        case ter_floor_hall:
          mvaddch(pc_offset[dim_y] + pos[dim_y] + 1,
                  pc_offset[dim_x] + pos[dim_x], '#');
          break;
        case ter_debug:
          mvaddch(pc_offset[dim_y] + pos[dim_y] + 1,
                  pc_offset[dim_x] + pos[dim_x], '*');
          break;
        case ter_stairs_up:
          mvaddch(pc_offset[dim_y] + pos[dim_y] + 1,
                  pc_offset[dim_x] + pos[dim_x], '<');
          break;
        case ter_stairs_down:
          mvaddch(pc_offset[dim_y] + pos[dim_y] + 1,
                  pc_offset[dim_x] + pos[dim_x], '>');
          break;
            case ter_water:
                attron(A_BOLD);
                attron(COLOR_PAIR(COLOR_BLUE));
                mvaddch(pos[dim_y] + 1, pos[dim_x], '!');
                attroff(COLOR_PAIR(COLOR_BLUE));
                attroff(A_BOLD);
                break;
            case ter_lava:
                attron(A_BOLD);
                attron(COLOR_PAIR(COLOR_RED));
                mvaddch(pos[dim_y] + 1, pos[dim_x], '!');
                attroff(COLOR_PAIR(COLOR_RED));
                attroff(A_BOLD);
                break;
        default:
 /* Use zero as an error symbol, since it stands out somewhat, and it's *
  * not otherwise used.                                                 */
          mvaddch(pc_offset[dim_y] + pos[dim_y] + 1,
                  pc_offset[dim_x] + pos[dim_x], '0');
        }
      }
      attroff(A_BOLD);
    }
  }

  refresh();
}

void io_display(dungeon_t *d)
{
  pair_t pos;
  uint32_t illuminated;
  uint32_t color;

  clear();
  for (pos[dim_y] = 0; pos[dim_y] < 21; pos[dim_y]++) {
    for (pos[dim_x] = 0; pos[dim_x] < 80; pos[dim_x]++) {
      if ((illuminated = is_illuminated(d->PC,
                                        d->io_offset[dim_y] + pos[dim_y],
                                        d->io_offset[dim_x] + pos[dim_x]))) {
        attron(A_BOLD);
      }
        if(d->map[d->io_offset[dim_y] + pos[dim_y]][d->io_offset[dim_x] + pos[dim_x]] == ter_projectile){
            attron(COLOR_PAIR(COLOR_CYAN));
            mvaddch(pos[dim_y] + 1, pos[dim_x], '*');
            attroff(COLOR_PAIR(COLOR_CYAN));
        }

        else if(d->map[d->io_offset[dim_y] + pos[dim_y]][d->io_offset[dim_x] + pos[dim_x]] == ter_spell){
            if ((illuminated = is_illuminated(d->PC,
                                              d->io_offset[dim_y] + pos[dim_y],
                                              d->io_offset[dim_x] + pos[dim_x]))) {
                attroff(A_BOLD);
            }
            attron(COLOR_PAIR(COLOR_GREEN));
            mvaddch(pos[dim_y] + 1, pos[dim_x], '*');
            attroff(COLOR_PAIR(COLOR_GREEN));
            if ((illuminated = is_illuminated(d->PC,
                                              d->io_offset[dim_y] + pos[dim_y],
                                              d->io_offset[dim_x] + pos[dim_x]))) {
                attron(A_BOLD);
            }
        }
      else if (d->character_map[d->io_offset[dim_y] + pos[dim_y]]
                          [d->io_offset[dim_x] + pos[dim_x]] &&
          can_see(d,
                  character_get_pos(d->PC),
                  character_get_pos(d->character_map[d->io_offset[dim_y] +
                                                     pos[dim_y]]
                                                    [d->io_offset[dim_x] +
                                                     pos[dim_x]]), 1, 0)) {

        attron(COLOR_PAIR((color =
                           d->character_map[d->io_offset[dim_y] + pos[dim_y]]
                                           [d->io_offset[dim_x] +
                                            pos[dim_x]]->get_color())));
        mvaddch(pos[dim_y] + 1, pos[dim_x],
                character_get_symbol(d->character_map[d->io_offset[dim_y] +
                                                      pos[dim_y]]
                                                     [d->io_offset[dim_x] +
                                                      pos[dim_x]]));
        attroff(COLOR_PAIR(color));
      } else if (d->objmap[d->io_offset[dim_y] + pos[dim_y]]
                          [d->io_offset[dim_x] + pos[dim_x]] &&
                 (d->objmap[d->io_offset[dim_y] + pos[dim_y]]
                           [d->io_offset[dim_x] + pos[dim_x]]->have_seen() ||
                  can_see(d, character_get_pos(d->PC), pos, 1, 0))) {
        attron(COLOR_PAIR(d->objmap[d->io_offset[dim_y] + pos[dim_y]]
                                   [d->io_offset[dim_x] +
                                    pos[dim_x]]->get_color()));
        mvaddch(pos[dim_y] + 1, pos[dim_x],
                d->objmap[d->io_offset[dim_y] + pos[dim_y]]
                         [d->io_offset[dim_x] + pos[dim_x]]->get_symbol());
        attroff(COLOR_PAIR(d->objmap[d->io_offset[dim_y] + pos[dim_y]]
                                    [d->io_offset[dim_x] +
                                     pos[dim_x]]->get_color()));
      } else {
        switch (pc_learned_terrain(d->PC,
                                   d->io_offset[dim_y] + pos[dim_y],
                                   d->io_offset[dim_x] + pos[dim_x])) {
        case ter_wall:
        case ter_wall_immutable:
        case ter_unknown:
          mvaddch(pos[dim_y] + 1, pos[dim_x], ' ');
          break;
        case ter_floor:
        case ter_floor_room:
          mvaddch(pos[dim_y] + 1, pos[dim_x], '.');
          break;
        case ter_floor_hall:
          mvaddch(pos[dim_y] + 1, pos[dim_x], '#');
          break;
        case ter_debug:
          mvaddch(pos[dim_y] + 1, pos[dim_x], '*');
          break;
        case ter_stairs_up:
          mvaddch(pos[dim_y] + 1, pos[dim_x], '<');
          break;
        case ter_stairs_down:
          mvaddch(pos[dim_y] + 1, pos[dim_x], '>');
          break;
          case ter_water:
            attron(COLOR_PAIR(COLOR_BLUE));
                mvaddch(pos[dim_y] + 1, pos[dim_x], '!');
            attroff(COLOR_PAIR(COLOR_BLUE));
            break;
          case ter_lava:
            attron(COLOR_PAIR(COLOR_RED));
                mvaddch(pos[dim_y] + 1, pos[dim_x], '!');
            attroff(COLOR_PAIR(COLOR_RED));
                break;
        default:
 /* Use zero as an error symbol, since it stands out somewhat, and it's *
  * not otherwise used.                                                 */
          mvaddch(pos[dim_y] + 1, pos[dim_x], '0');
        }
      }
      if (illuminated) {
        attroff(A_BOLD);
      }
    }
  }

  mvprintw(23, 0, "PC position is (%3d,%2d); offset is (%3d,%2d).",
           character_get_x(d->PC), character_get_y(d->PC),
           d->io_offset[dim_x], d->io_offset[dim_y]);

  io_print_message_queue(0, 0);

  refresh();
}

void io_look_mode(dungeon_t *d)
{
  int32_t key;

  do {
    if ((key = getch()) == 27 /* ESC */) {
      io_calculate_offset(d);
      io_display(d);
      return;
    }
    
    switch (key) {
    case '1':
    case 'b':
    case KEY_END:
    case '2':
    case 'j':
    case KEY_DOWN:
    case '3':
    case 'n':
    case KEY_NPAGE:
      if (d->io_offset[dim_y] != 84) {
        d->io_offset[dim_y] += 7;
      }
      break;
    case '4':
    case 'h':
    case KEY_LEFT:
    case '5':
    case ' ':
    case KEY_B2:
    case '6':
    case 'l':
    case KEY_RIGHT:
      break;
    case '7':
    case 'y':
    case KEY_HOME:
    case '8':
    case 'k':
    case KEY_UP:
    case '9':
    case 'u':
    case KEY_PPAGE:
      if (d->io_offset[dim_y]) {
        d->io_offset[dim_y] -= 7;
      }
      break;
    }
    switch (key) {
    case '1':
    case 'b':
    case KEY_END:
    case '4':
    case 'h':
    case KEY_LEFT:
    case '7':
    case 'y':
    case KEY_HOME:
      if (d->io_offset[dim_x]) {
        d->io_offset[dim_x] -= 20;
      }
      break;
    case '2':
    case 'j':
    case KEY_DOWN:
    case '5':
    case ' ':
    case KEY_B2:
    case '8':
    case 'k':
    case KEY_UP:
      break;
    case '3':
    case 'n':
    case KEY_NPAGE:
    case '6':
    case 'l':
    case KEY_RIGHT:
    case '9':
    case 'u':
    case KEY_PPAGE:
      if (d->io_offset[dim_x] != 80) {
        d->io_offset[dim_x] += 20;
      }
      break;
    }
    io_display(d);
  } while (1);
}

void io_display_monster_list(dungeon_t *d)
{
  mvprintw(11, 33, " HP:    XXXXX ");
  mvprintw(12, 33, " Speed: XXXXX ");
  mvprintw(14, 27, " Hit any key to continue. ");
  refresh();
  getch();
}

uint32_t io_teleport_pc(dungeon_t *d)
{
  /* Just for fun. */
  pair_t dest;

  do {
    dest[dim_x] = rand_range(1, DUNGEON_X - 2);
    dest[dim_y] = rand_range(1, DUNGEON_Y - 2);
  } while (charpair(dest));

  d->character_map[character_get_y(d->PC)][character_get_x(d->PC)] = NULL;
  d->character_map[dest[dim_y]][dest[dim_x]] = d->PC;

  character_set_y(d->PC, dest[dim_y]);
  character_set_x(d->PC, dest[dim_x]);

  if (mappair(dest) < ter_floor) {
    mappair(dest) = ter_floor;
  }

  pc_observe_terrain(d->PC, d);
  io_calculate_offset(d);
  dijkstra(d);
  dijkstra_tunnel(d);

  return 0;
}
/* Adjectives to describe our monsters */
static const char *adjectives[] = {
  "A menacing ",
  "A threatening ",
  "A horrifying ",
  "An intimidating ",
  "An aggressive ",
  "A frightening ",
  "A terrifying ",
  "A terrorizing ",
  "An alarming ",
  "A frightening ",
  "A dangerous ",
  "A glowering ",
  "A glaring ",
  "A scowling ",
  "A chilling ",
  "A scary ",
  "A creepy ",
  "An eerie ",
  "A spooky ",
  "A slobbering ",
  "A drooling ",
  " A horrendous ",
  "An unnerving ",
  "A cute little ",  /* Even though they're trying to kill you, */
  "A teeny-weenie ", /* they can still be cute!                 */
  "A fuzzy ",
  "A fluffy white ",
  "A kawaii ",       /* For our otaku */
  "Hao ke ai de "    /* And for our Chinese */
  /* And there's one special case (see below) */
};

static void io_scroll_monster_list(char (*s)[40], uint32_t count)
{
  uint32_t offset;
  uint32_t i;

  offset = 0;

  while (1) {
    for (i = 0; i < 13; i++) {
      mvprintw(i + 6, 19, " %-40s ", s[i + offset]);
    }
    switch (getch()) {
    case KEY_UP:
      if (offset) {
        offset--;
      }
      break;
    case KEY_DOWN:
      if (offset < (count - 13)) {
        offset++;
      }
      break;
    case 27:
      return;
    }

  }
}

static bool is_vowel(const char c)
{
  return (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' ||
          c == 'A' || c == 'E' || c == 'I' || c == 'O' || c == 'U');
}

static void io_list_monsters_display(dungeon_t *d,
                                     character **c,
                                     uint32_t count)
{
  uint32_t i;
  char (*s)[40]; /* pointer to array of 40 char */
  (void) adjectives;

  s = (char (*)[40]) malloc(count * sizeof (*s));

  mvprintw(3, 19, " %-40s ", "");
  /* Borrow the first element of our array for this string: */
  snprintf(s[0], 40, "You know of %d monsters:", count);
  mvprintw(4, 19, " %-40s ", s);
  mvprintw(5, 19, " %-40s ", "");

  for (i = 0; i < count; i++) {
    snprintf(s[i], 40, "%3s%s (%c): %2d %s by %2d %s",
             (is_vowel(character_get_name(c[i])[0]) ? "An " : "A "),
             character_get_name(c[i]),
             character_get_symbol(c[i]),
             abs(character_get_y(c[i]) - character_get_y(d->PC)),
             ((character_get_y(c[i]) - character_get_y(d->PC)) <= 0 ?
              "North" : "South"),
             abs(character_get_x(c[i]) - character_get_x(d->PC)),
             ((character_get_x(c[i]) - character_get_x(d->PC)) <= 0 ?
              "East" : "West"));
    if (count <= 13) {
      /* Handle the non-scrolling case right here. *
       * Scrolling in another function.            */
      mvprintw(i + 6, 19, " %-40s ", s[i]);
    }
  }

  if (count <= 13) {
    mvprintw(count + 6, 19, " %-40s ", "");
    mvprintw(count + 7, 19, " %-40s ", "Hit escape to continue.");
    while (getch() != 27 /* escape */)
      ;
  } else {
    mvprintw(19, 19, " %-40s ", "");
    mvprintw(20, 19, " %-40s ",
             "Arrows to scroll, escape to continue.");
    io_scroll_monster_list(s, count);
  }

  free(s);
}

static int compare_monster_distance(const void *v1, const void *v2)
{
  const character *const *c1 = (character **) v1;
  const character *const *c2 = (character **) v2;

  return (dungeon->pc_distance[character_get_y(*c1)][character_get_x(*c1)] -
          dungeon->pc_distance[character_get_y(*c2)][character_get_x(*c2)]);
}

static void io_list_monsters(dungeon_t *d)
{
  character **c;
  uint32_t x, y, count;

  c = (character **) malloc(d->num_monsters * sizeof (*c));

  /* Get a linear list of monsters */
  for (count = 0, y = 1; y < DUNGEON_Y - 1; y++) {
    for (x = 1; x < DUNGEON_X - 1; x++) {
      if (d->character_map[y][x] && d->character_map[y][x] != d->PC) {
        c[count++] = d->character_map[y][x];
      }
    }
  }

  /* Sort it by distance from PC */
  dungeon = d;
  qsort(c, count, sizeof (*c), compare_monster_distance);

  /* Display it */
  io_list_monsters_display(d, c, count);
  free(c);

  /* And redraw the dungeon */
  io_display(d);
}

void io_display_ch(dungeon_t *d)
{
  mvprintw(11, 33, " HP:    %5d ", d->PC->hp);
  mvprintw(12, 33, " Speed: %5d ", d->PC->speed);
  mvprintw(14, 27, " Hit any key to continue. ");
  refresh();
  getch();
  io_display(d);
}

void io_object_to_string(object *o, char *s, uint32_t size)
{
  if (o) {
    snprintf(s, size, "%s (sp: %d, dmg: %d+%dd%d)",
             o->get_name(), o->get_speed(), o->get_damage_base(),
             o->get_damage_number(), o->get_damage_sides());
  } else {
    *s = '\0';
  }
}

uint32_t io_wear_eq(dungeon_t *d)
{
  uint32_t i, key;
  char s[61];

  for (i = 0; i < MAX_INVENTORY; i++) {
    /* We'll write 12 lines, 10 of inventory, 1 blank, and 1 prompt. *
     * We'll limit width to 60 characters, so very long object names *
     * will be truncated.  In an 80x24 terminal, this gives offsets  *
     * at 10 x and 6 y to start printing things.  Same principal in  *
     * other functions, below.                                       */
    io_object_to_string(d->PC->in[i], s, 61);
    mvprintw(i + 6, 10, " %c) %-55s ", '0' + i, s);
  }
  mvprintw(16, 10, " %-58s ", "");
  mvprintw(17, 10, " %-58s ", "Wear which item (ESC to cancel)?");
  refresh();

  while (1) {
    if ((key = getch()) == 27 /* ESC */) {
      io_display(d);
      return 1;
    }

    if (key < '0' || key > '9') {
      if (isprint(key)) {
        snprintf(s, 61, "Invalid input: '%c'.  Enter 0-9 or ESC to cancel.",
                 key);
        mvprintw(18, 10, " %-58s ", s);
      } else {
        mvprintw(18, 10, " %-58s ",
                 "Invalid input.  Enter 0-9 or ESC to cancel.");
      }
      refresh();
      continue;
    }

    if (!d->PC->in[key - '0']) {
      mvprintw(18, 10, " %-58s ", "Empty inventory slot.  Try again.");
      continue;
    }

    if (!d->PC->wear_in(key - '0')) {
      return 0;
    }

    snprintf(s, 61, "Can't wear %s.  Try again.",
             d->PC->in[key - '0']->get_name());
    mvprintw(18, 10, " %-58s ", s);
    refresh();
  }

  return 1;
}

void io_display_in(dungeon_t *d)
{
  uint32_t i;
  char s[61];

  for (i = 0; i < MAX_INVENTORY; i++) {
    io_object_to_string(d->PC->in[i], s, 61);
    mvprintw(i + 7, 10, " %c) %-55s ", '0' + i, s);
  }

  mvprintw(17, 10, " %-58s ", "");
  mvprintw(18, 10, " %-58s ", "Hit any key to continue.");

  refresh();

  getch();

  io_display(d);
}

uint32_t io_remove_eq(dungeon_t *d)
{
  uint32_t i, key;
  char s[61], t[61];

  for (i = 0; i < num_eq_slots; i++) {
    sprintf(s, "[%s]", eq_slot_name[i]);
    io_object_to_string(d->PC->eq[i], t, 61);
    mvprintw(i + 5, 10, " %c %-9s) %-45s ", 'a' + i, s, t);
  }
  mvprintw(17, 10, " %-58s ", "");
  mvprintw(18, 10, " %-58s ", "Take off which item (ESC to cancel)?");
  refresh();

  while (1) {
    if ((key = getch()) == 27 /* ESC */) {
      io_display(d);
      return 1;
    }

    if (key < 'a' || key > 'l') {
      if (isprint(key)) {
        snprintf(s, 61, "Invalid input: '%c'.  Enter a-l or ESC to cancel.",
                 key);
        mvprintw(18, 10, " %-58s ", s);
      } else {
        mvprintw(18, 10, " %-58s ",
                 "Invalid input.  Enter a-l or ESC to cancel.");
      }
      refresh();
      continue;
    }

    if (!d->PC->eq[key - 'a']) {
      mvprintw(18, 10, " %-58s ", "Empty equipment slot.  Try again.");
      continue;
    }

    if (!d->PC->remove_eq(key - 'a')) {
      return 0;
    }

    snprintf(s, 61, "Can't take off %s.  Try again.",
             d->PC->eq[key - 'a']->get_name());
    mvprintw(19, 10, " %-58s ", s);
  }

  return 1;
}

void io_display_eq(dungeon_t *d)
{
  uint32_t i;
  char s[61], t[61];

  for (i = 0; i < num_eq_slots; i++) {
    sprintf(s, "[%s]", eq_slot_name[i]);
    io_object_to_string(d->PC->eq[i], t, 61);
    mvprintw(i + 5, 10, " %c %-9s) %-45s ", 'a' + i, s, t);
  }
  mvprintw(17, 10, " %-58s ", "");
  mvprintw(18, 10, " %-58s ", "Hit any key to continue.");

  refresh();

  getch();

  io_display(d);
}

uint32_t io_drop_in(dungeon_t *d)
{
  uint32_t i, key;
  char s[61];

  for (i = 0; i < MAX_INVENTORY; i++) {
      mvprintw(i + 6, 10, " %c) %-55s ", '0' + i,
               d->PC->in[i] ? d->PC->in[i]->get_name() : "");
  }
  mvprintw(16, 10, " %-58s ", "");
  mvprintw(17, 10, " %-58s ", "Drop which item (ESC to cancel)?");
  refresh();

  while (1) {
    if ((key = getch()) == 27 /* ESC */) {
      io_display(d);
      return 1;
    }

    if (key < '0' || key > '9') {
      if (isprint(key)) {
        snprintf(s, 61, "Invalid input: '%c'.  Enter 0-9 or ESC to cancel.",
                 key);
        mvprintw(18, 10, " %-58s ", s);
      } else {
        mvprintw(18, 10, " %-58s ",
                 "Invalid input.  Enter 0-9 or ESC to cancel.");
      }
      refresh();
      continue;
    }

    if (!d->PC->in[key - '0']) {
      mvprintw(18, 10, " %-58s ", "Empty inventory slot.  Try again.");
      continue;
    }

    if (!d->PC->drop_in(d, key - '0')) {
      return 0;
    }

    snprintf(s, 61, "Can't drop %s.  Try again.",
             d->PC->in[key - '0']->get_name());
    mvprintw(18, 10, " %-58s ", s);
    refresh();
  }

  return 1;
}

static uint32_t io_display_obj_info(object *o)
{
  char s[80];
  uint32_t i, l;
  uint32_t n;

  for (i = 0; i < 79; i++) {
    s[i] = ' ';
  }
  s[79] = '\0';

  l = strlen(o->get_description());
  for (i = n = 0; i < l; i++) {
    if (o->get_description()[i] == '\n') {
      n++;
    }
  }

  for (i = 0; i < n + 4; i++) {
    mvprintw(i, 0, s);
  }

  io_object_to_string(o, s, 80);
  mvprintw(1, 0, s);
  mvprintw(3, 0, o->get_description());

  mvprintw(n + 5, 0, "Hit any key to continue.");

  refresh();
  getch();

  return 0;  
}

static uint32_t io_inspect_eq(dungeon_t *d);

static uint32_t io_inspect_in(dungeon_t *d)
{
  uint32_t i, key;
  char s[61];

  for (i = 0; i < MAX_INVENTORY; i++) {
    io_object_to_string(d->PC->in[i], s, 61);
    mvprintw(i + 6, 10, " %c) %-55s ", '0' + i,
             d->PC->in[i] ? d->PC->in[i]->get_name() : "");
  }
  mvprintw(16, 10, " %-58s ", "");
  mvprintw(17, 10, " %-58s ", "Inspect which item (ESC to cancel, '/' for equipment)?");
  refresh();

  while (1) {
    if ((key = getch()) == 27 /* ESC */) {
      io_display(d);
      return 1;
    }

    if (key == '/') {
      io_display(d);
      io_inspect_eq(d);
      return 1;
    }

    if (key < '0' || key > '9') {
      if (isprint(key)) {
        snprintf(s, 61, "Invalid input: '%c'.  Enter 0-9 or ESC to cancel.",
                 key);
        mvprintw(18, 10, " %-58s ", s);
      } else {
        mvprintw(18, 10, " %-58s ",
                 "Invalid input.  Enter 0-9 or ESC to cancel.");
      }
      refresh();
      continue;
    }

    if (!d->PC->in[key - '0']) {
      mvprintw(18, 10, " %-58s ", "Empty inventory slot.  Try again.");
      refresh();
      continue;
    }

    io_display(d);
    io_display_obj_info(d->PC->in[key - '0']);
    io_display(d);
    return 1;
  }

  return 1;
}

static uint32_t io_inspect_eq(dungeon_t *d)
{
  uint32_t i, key;
  char s[61], t[61];

  for (i = 0; i < num_eq_slots; i++) {
    sprintf(s, "[%s]", eq_slot_name[i]);
    io_object_to_string(d->PC->eq[i], t, 61);
    mvprintw(i + 5, 10, " %c %-9s) %-45s ", 'a' + i, s, t);
  }
  mvprintw(17, 10, " %-58s ", "");
  mvprintw(18, 10, " %-58s ", "Inspect which item (ESC to cancel, '/' for inventory)?");
  refresh();

  while (1) {
    if ((key = getch()) == 27 /* ESC */) {
      io_display(d);
      return 1;
    }

    if (key == '/') {
      io_display(d);
      io_inspect_in(d);
      return 1;
    }

    if (key < 'a' || key > 'l') {
      if (isprint(key)) {
        snprintf(s, 61, "Invalid input: '%c'.  Enter a-l or ESC to cancel.",
                 key);
        mvprintw(18, 10, " %-58s ", s);
      } else {
        mvprintw(18, 10, " %-58s ",
                 "Invalid input.  Enter a-l or ESC to cancel.");
      }
      refresh();
      continue;
    }

    if (!d->PC->eq[key - 'a']) {
      mvprintw(18, 10, " %-58s ", "Empty equipment slot.  Try again.");
      continue;
    }

    io_display(d);
    io_display_obj_info(d->PC->eq[key - 'a']);
    io_display(d);
    return 1;
  }

  return 1;
}

uint32_t io_expunge_in(dungeon_t *d)
{
  uint32_t i, key;
  char s[61];

  for (i = 0; i < MAX_INVENTORY; i++) {
    /* We'll write 12 lines, 10 of inventory, 1 blank, and 1 prompt. *
     * We'll limit width to 60 characters, so very long object names *
     * will be truncated.  In an 80x24 terminal, this gives offsets  *
     * at 10 x and 6 y to start printing things.                     */
      mvprintw(i + 6, 10, " %c) %-55s ", '0' + i,
               d->PC->in[i] ? d->PC->in[i]->get_name() : "");
  }
  mvprintw(16, 10, " %-58s ", "");
  mvprintw(17, 10, " %-58s ", "Destroy which item (ESC to cancel)?");
  refresh();

  while (1) {
    if ((key = getch()) == 27 /* ESC */) {
      io_display(d);
      return 1;
    }

    if (key < '0' || key > '9') {
      if (isprint(key)) {
        snprintf(s, 61, "Invalid input: '%c'.  Enter 0-9 or ESC to cancel.",
                 key);
        mvprintw(18, 10, " %-58s ", s);
      } else {
        mvprintw(18, 10, " %-58s ",
                 "Invalid input.  Enter 0-9 or ESC to cancel.");
      }
      refresh();
      continue;
    }

    if (!d->PC->in[key - '0']) {
      mvprintw(18, 10, " %-58s ", "Empty inventory slot.  Try again.");
      continue;
    }

    if (!d->PC->destroy_in(key - '0')) {
      io_display(d);

      return 1;
    }

    snprintf(s, 61, "Can't destroy %s.  Try again.",
             d->PC->in[key - '0']->get_name());
    mvprintw(18, 10, " %-58s ", s);
    refresh();
  }

  return 1;
}

void io_handle_input(dungeon_t *d)
{
  uint32_t fail_code;
  int key;
  fd_set readfs;
  struct timeval tv;

  do {
    do{
      FD_ZERO(&readfs);
      FD_SET(STDIN_FILENO, &readfs);

      tv.tv_sec = 0;
      tv.tv_usec = 125000; /* An eigth of a second */

      io_redisplay_visible_monsters(d);
    } while (!select(STDIN_FILENO + 1, &readfs, NULL, NULL, &tv));

    switch (key = getch()) {
    case '7':
    case 'y':
    case KEY_HOME:
      fail_code = move_pc(d, 7);
      break;
    case '8':
    case 'k':
    case KEY_UP:
      fail_code = move_pc(d, 8);
      break;
    case '9':
    case 'u':
    case KEY_PPAGE:
      fail_code = move_pc(d, 9);
      break;
    case '6':
    case 'l':
    case KEY_RIGHT:
      fail_code = move_pc(d, 6);
      break;
    case '3':
    case 'n':
    case KEY_NPAGE:
      fail_code = move_pc(d, 3);
      break;
    case '2':
    case 'j':
    case KEY_DOWN:
      fail_code = move_pc(d, 2);
      break;
    case '1':
    case 'b':
    case KEY_END:
      fail_code = move_pc(d, 1);
      break;
    case '4':
    case 'h':
    case KEY_LEFT:
      fail_code = move_pc(d, 4);
      break;
    case '5':
    case ' ':
    case KEY_B2:
      fail_code = 0;
      break;
    case '>':
      fail_code = move_pc(d, '>');
      break;
    case '<':
      fail_code = move_pc(d, '<');
      break;
    case 'Q':
      d->quit = 1;
      fail_code = 0;
      break;
#if 0
    case 'T':
      /* New command.  Display the distances for tunnelers.             */
      io_display_tunnel(d);
      fail_code = 1;
      break;
    case 'D':
      /* New command.  Display the distances for non-tunnelers.         */
      io_display_distance(d);
      fail_code = 1;
      break;
    case 'H':
      /* New command.  Display the hardnesses.                          */
      io_display_hardness(d);
      fail_code = 1;
      break;
    case 's':
      /* New command.  Return to normal display after displaying some   *
       * special screen.                                                */
      io_display(d);
      fail_code = 1;
      break;
#endif
    case 'L':
      io_look_mode(d);
      fail_code = 1;
      break;
    case 'g':
      /* Teleport the PC to a random place in the dungeon.              */
      io_teleport_pc(d);
      fail_code = 0;
      break;
    case 'm':
      io_list_monsters(d);
      fail_code = 1;
      break;
      break;
    case 'w':
      fail_code = io_wear_eq(d);
      break;
    case 't':
      fail_code = io_remove_eq(d);
      break;
    case 'd':
      fail_code = io_drop_in(d);
      break;
    case 'x':
      fail_code = io_expunge_in(d);
      break;
    case 'i':
      io_display_in(d);
      fail_code = 1;
      break;
    case 'e':
      io_display_eq(d);
      fail_code = 1;
      break;
    case 'c':
      io_display_ch(d);
      fail_code = 1;
      break;
    case 'I':
      io_inspect_in(d);
      fail_code = 1;
      break;
    case 'q':
      /* Demonstrate use of the message queue.  You can use this for *
       * printf()-style debugging (though gdb is probably a better   *
       * option.  Not that it matterrs, but using this command will  *
       * waste a turn.  Set fail_code to 1 and you should be able to *
       * figure out why I did it that way.                           */
      io_queue_message("This is the first message.");
      io_queue_message("Since there are multiple messages, "
                       "you will see \"more\" prompts.");
      io_queue_message("You can use any key to advance through messages.");
      io_queue_message("Normal gameplay will not resume until the queue "
                       "is empty.");
      io_queue_message("Long lines will be truncated, not wrapped.");
      io_queue_message("io_queue_message() is variadic and handles "
                       "all printf() conversion specifiers.");
      io_queue_message("Did you see %s?", "what I did there");
      io_queue_message("When the last message is displayed, there will "
                       "be no \"more\" prompt.");
      io_queue_message("Have fun!  And happy printing!");
      fail_code = 0;
      break;
        case 'a':
            if(d->PC->eq[2] == NULL){
                io_queue_message("You do not have a ranged object equipped");
                fail_code = 1;
            }
            else {
                //io_queue_message("Choose a character letter to attack");
                key = getch();
                int i, j, attacked = 0;
                for (i = d->PC->position[dim_y] - 10; i < d->PC->position[dim_y] + 10; i++) {
                    for (j = d->PC->position[dim_x] - 10; j < d->PC->position[dim_x] + 10; j++) {

                        //If character matches user input and is in illuminated area
                        if (i >= 0 || j >= 0) {
                            if (d->character_map[i][j] != NULL) {
                                if (d->character_map[i][j]->symbol == key && is_illuminated(d->PC, i, j) && attacked == 0) {
                                    uint dam = d->PC->eq[2]->roll_dice();
                                    character *c = d->character_map[i][j];

                                    int proj_y = d->PC->position[dim_y], proj_x = d->PC->position[dim_x];
                                    terrain_type_t temp = d->map[proj_y][proj_x];
                                    d->map[proj_y][proj_x] = ter_projectile;
                                    while (proj_y != i || proj_x != j) {
                                        d->map[proj_y][proj_x] = temp;
                                        if (proj_y < i && proj_x < j) {
                                            proj_y++;
                                            proj_x++;
                                        } else if (proj_y < i && proj_x > j) {
                                            proj_y++;
                                            proj_x--;
                                        } else if (proj_y > i && proj_x < j) {
                                            proj_y--;
                                            proj_x++;
                                        } else if (proj_y > i && proj_x > j) {
                                            proj_y--;
                                            proj_x--;
                                        } else if (proj_y < i) {
                                            proj_y++;
                                        } else if (proj_y > i) {
                                            proj_y--;
                                        } else if (proj_x < j) {
                                            proj_x++;
                                        } else {
                                            proj_x--;
                                        }
                                        temp = d->map[proj_y][proj_x];
                                        d->map[proj_y][proj_x] = ter_projectile;
                                        io_display(d);
                                        usleep(83333);
                                    }
                                    d->map[proj_y][proj_x] = temp;

                                    if (d->character_map[i][j]->hp <= dam) {
                                        io_queue_message("The %s dies.", c->name);
                                        c->hp = 0;
                                        c->alive = 0;
                                        character_increment_dkills(d->PC);
                                        character_increment_ikills(d->PC, (character_get_dkills(c) +
                                                                           character_get_ikills(c)));
                                        if (c != d->PC) {
                                            d->num_monsters--;
                                        }
                                        charpair(c->position) = NULL;
                                        fail_code = 0;
                                    } else {
                                        io_queue_message("You deal %d damage to the %s", dam, c->name);
                                        d->character_map[i][j]->hp = d->character_map[i][j]->hp - dam;
                                        fail_code = 0;
                                    }
                                    attacked = 1;
                                }
                            }
                        }
                    }
                }
            }
            break;
        case 's':
            //io_queue_message("Choose a character letter to attack");
            key = getch();
            int i, j, attacked, target_y, target_x;
            attacked = 0;
            //Find the main target and deal full damage of spell
            for (i = d->PC->position[dim_y] - 10; i < d->PC->position[dim_y] + 10; i++) {
                for (j = d->PC->position[dim_x] - 10; j < d->PC->position[dim_x] + 10; j++) {

                    //If character matches user input and is in illuminated area
                    if (i >= 0 || j >= 0) {
                        if (d->character_map[i][j] != NULL) {
                            if (d->character_map[i][j]->symbol == key && is_illuminated(d->PC, i, j) && attacked == 0) {
                                uint dam = rand() % 120 + 40;
                                character *c = d->character_map[i][j];

                                int proj_y = d->PC->position[dim_y], proj_x = d->PC->position[dim_x];
                                terrain_type_t temp = d->map[proj_y][proj_x];
                                d->map[proj_y][proj_x] = ter_spell;
                                while (proj_y != i || proj_x != j) {
                                    d->map[proj_y][proj_x] = temp;
                                    if (proj_y < i && proj_x < j) {
                                        proj_y++;
                                        proj_x++;
                                    } else if (proj_y < i && proj_x > j) {
                                        proj_y++;
                                        proj_x--;
                                    } else if (proj_y > i && proj_x < j) {
                                        proj_y--;
                                        proj_x++;
                                    } else if (proj_y > i && proj_x > j) {
                                        proj_y--;
                                        proj_x--;
                                    } else if (proj_y < i) {
                                        proj_y++;
                                    } else if (proj_y > i) {
                                        proj_y--;
                                    } else if (proj_x < j) {
                                        proj_x++;
                                    } else {
                                        proj_x--;
                                    }


                                    temp = d->map[proj_y][proj_x];
                                    d->map[proj_y][proj_x] = ter_spell;
                                    io_display(d);
                                    usleep(83333);
                                }
                                d->map[proj_y][proj_x] = temp;

                                if (d->character_map[i][j]->hp <= dam) {
                                    io_queue_message("The %s dies.", c->name);
                                    c->hp = 0;
                                    c->alive = 0;
                                    character_increment_dkills(d->PC);
                                    character_increment_ikills(d->PC, (character_get_dkills(c) +
                                                                       character_get_ikills(c)));
                                    if (c != d->PC) {
                                        d->num_monsters--;
                                    }
                                    charpair(c->position) = NULL;
                                    fail_code = 0;
                                } else {
                                    io_queue_message("You deal %d damage to the %s", dam, c->name);
                                    d->character_map[i][j]->hp = d->character_map[i][j]->hp - dam;
                                    fail_code = 0;
                                }
                                attacked = 1;
                                target_y = i;
                                target_x = j;
                            }
                            else {
                                uint dam = rand() % 60 + 40;
                                character *c = d->character_map[i][j];

                                if (d->character_map[i][j]->hp <= dam) {
                                    c->hp = 0;
                                    c->alive = 0;
                                    character_increment_dkills(d->PC);
                                    character_increment_ikills(d->PC, (character_get_dkills(c) +
                                                                       character_get_ikills(c)));
                                    if (c != d->PC) {
                                        d->num_monsters--;
                                    }
                                    charpair(c->position) = NULL;
                                    fail_code = 0;
                                } else {
                                    d->character_map[i][j]->hp = d->character_map[i][j]->hp - dam;
                                    fail_code = 0;
                                }
                            }
                        }
                    }
                }
            }
            terrain_type_t temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
            //Damage at half for monsters in a two cell radius of main target
            if(target_y + 2 <= DUNGEON_Y && target_y - 2 >= 0 && target_x + 2 <= DUNGEON_X && target_x - 2 >= 0) {
                for (i = 0; i < 2; i++) {
                    if (i == 0) {
                        temp1 = d->map[target_y + 1][target_x];
                        temp2 = d->map[target_y - 1][target_x];
                        temp3 = d->map[target_y][target_x + 1];
                        temp4 = d->map[target_y][target_x - 1];
                        d->map[target_y + 1][target_x] = ter_spell;
                        d->map[target_y - 1][target_x] = ter_spell;
                        d->map[target_y][target_x + 1] = ter_spell;
                        d->map[target_y][target_x - 1] = ter_spell;
                    } else {
                        d->map[target_y + 1][target_x] = temp1;
                        d->map[target_y - 1][target_x] = temp2;
                        d->map[target_y][target_x + 1] = temp3;
                        d->map[target_y][target_x - 1] = temp4;
                        temp1 = d->map[target_y - 2][target_x];
                        temp2 = d->map[target_y - 1][target_x + 1];
                        temp3 = d->map[target_y][target_x + 2];
                        temp4 = d->map[target_y + 1][target_x + 1];
                        temp5 = d->map[target_y + 2][target_x];
                        temp6 = d->map[target_y + 1][target_x - 1];
                        temp7 = d->map[target_y][target_x - 2];
                        temp8 = d->map[target_y - 1][target_x - 1];
                        d->map[target_y - 2][target_x] = ter_spell;
                        d->map[target_y - 1][target_x + 1] = ter_spell;
                        d->map[target_y][target_x + 2] = ter_spell;
                        d->map[target_y + 1][target_x + 1] = ter_spell;
                        d->map[target_y + 2][target_x] = ter_spell;
                        d->map[target_y + 1][target_x - 1] = ter_spell;
                        d->map[target_y][target_x - 2] = ter_spell;
                        d->map[target_y - 1][target_x - 1] = ter_spell;
                    }
                    io_display(d);
                    usleep(120000);
                }
                d->map[target_y - 2][target_x] = temp1;
                d->map[target_y - 1][target_x + 1] = temp2;
                d->map[target_y][target_x + 2] = temp3;
                d->map[target_y + 1][target_x + 1] = temp4;
                d->map[target_y + 2][target_x] = temp5;
                d->map[target_y + 1][target_x - 1] = temp6;
                d->map[target_y][target_x - 2] = temp7;
                d->map[target_y - 1][target_x - 1] = temp8;
            }
            break;
    default:
      /* Also not in the spec.  It's not always easy to figure out what *
       * key code corresponds with a given keystroke.  Print out any    *
       * unhandled key here.  Not only does it give a visual error      *
       * indicator, but it also gives an integer value that can be used *
       * for that key in this (or other) switch statements.  Printed in *
       * octal, with the leading zero, because ncurses.h lists codes in *
       * octal, thus allowing us to do reverse lookups.  If a key has a *
       * name defined in the header, you can use the name here, else    *
       * you can directly use the octal value.                          */
      mvprintw(0, 0, "Unbound key: %#o ", key);
      fail_code = 1;
    }
  } while (fail_code);
}
