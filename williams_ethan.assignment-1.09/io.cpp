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
      if (d->character_map[d->io_offset[dim_y] + pos[dim_y]]
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

    mvprintw(24, 0, "HP: %d   Speed: %d", d->PC->hp, d->PC->speed);

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

//Shifts inventory items to cover equipped, dropped, or deleted item
void shift_inventory(dungeon_t *d){
    int i;
    for(i = 0; i < 9; i++){
        if(d->PC->inventory[i] == NULL && d->PC->inventory[i+1] != NULL){
            d->PC->inventory[i] = d->PC->inventory[i+1];
            d->PC->inventory[i+1] = NULL;
        }
    }
}

//Equips item in appropriate slot based on type
void equip(dungeon_t *d, int inventory_index){
    int k;
    if(d->PC->inventory[inventory_index]->get_type() == objtype_WEAPON){
        if(d->PC->equipment[0] == NULL){
            d->PC->equipment[0] = d->PC->inventory[inventory_index];
            d->PC->inventory[inventory_index] = NULL;
            shift_inventory(d);
        }
        else{
            object *a = d->PC->equipment[0];
            d->PC->equipment[0] = d->PC->inventory[inventory_index];
            d->PC->inventory[inventory_index] = NULL;
            shift_inventory(d);
            for(k = 0; k < 10; k++){
                if(d->PC->inventory[k] == NULL){
                    d->PC->inventory[k] = a;
                    break;
                }
            }
        }
        d->PC->speed += d->PC->equipment[0]->get_speed();
    }
    else if(d->PC->inventory[inventory_index]->get_type() == objtype_OFFHAND){
        if(d->PC->equipment[1] == NULL){
            d->PC->equipment[1] = d->PC->inventory[inventory_index];
            d->PC->inventory[inventory_index] = NULL;
            shift_inventory(d);
        }
        else{
            object *a = d->PC->equipment[1];
            d->PC->equipment[1] = d->PC->inventory[inventory_index];
            d->PC->inventory[inventory_index] = NULL;
            shift_inventory(d);
            for(k = 0; k < 10; k++){
                if(d->PC->inventory[k] == NULL){
                    d->PC->inventory[k] = a;
                    break;
                }
            }
        }
        d->PC->speed += d->PC->equipment[1]->get_speed();
    }
    else if(d->PC->inventory[inventory_index]->get_type() == objtype_RANGED){
        if(d->PC->equipment[2] == NULL){
            d->PC->equipment[2] = d->PC->inventory[inventory_index];
            d->PC->inventory[inventory_index] = NULL;
            shift_inventory(d);
        }
        else{
            object *a = d->PC->equipment[2];
            d->PC->equipment[2] = d->PC->inventory[inventory_index];
            d->PC->inventory[inventory_index] = NULL;
            shift_inventory(d);
            for(k = 0; k < 10; k++){
                if(d->PC->inventory[k] == NULL){
                    d->PC->inventory[k] = a;
                    break;
                }
            }
        }
        d->PC->speed += d->PC->equipment[2]->get_speed();
    }
    else if(d->PC->inventory[inventory_index]->get_type() == objtype_ARMOR){
        if(d->PC->equipment[3] == NULL){
            d->PC->equipment[3] = d->PC->inventory[inventory_index];
            d->PC->inventory[inventory_index] = NULL;
            shift_inventory(d);
        }
        else{
            object *a = d->PC->equipment[3];
            d->PC->equipment[3] = d->PC->inventory[inventory_index];
            d->PC->inventory[inventory_index] = NULL;
            shift_inventory(d);
            for(k = 0; k < 10; k++){
                if(d->PC->inventory[k] == NULL){
                    d->PC->inventory[k] = a;
                    break;
                }
            }
        }
        d->PC->speed += d->PC->equipment[3]->get_speed();
    }
    else if(d->PC->inventory[inventory_index]->get_type() == objtype_HELMET){
        if(d->PC->equipment[4] == NULL){
            d->PC->equipment[4] = d->PC->inventory[inventory_index];
            d->PC->inventory[inventory_index] = NULL;
            shift_inventory(d);
        }
        else{
            object *a = d->PC->equipment[4];
            d->PC->equipment[4] = d->PC->inventory[inventory_index];
            d->PC->inventory[inventory_index] = NULL;
            shift_inventory(d);
            for(k = 0; k < 10; k++){
                if(d->PC->inventory[k] == NULL){
                    d->PC->inventory[k] = a;
                    break;
                }
            }
        }
        d->PC->speed += d->PC->equipment[4]->get_speed();
    }
    else if(d->PC->inventory[inventory_index]->get_type() == objtype_CLOAK){
        if(d->PC->equipment[5] == NULL){
            d->PC->equipment[5] = d->PC->inventory[inventory_index];
            d->PC->inventory[inventory_index] = NULL;
            shift_inventory(d);
        }
        else{
            object *a = d->PC->equipment[5];
            d->PC->equipment[5] = d->PC->inventory[inventory_index];
            d->PC->inventory[inventory_index] = NULL;
            shift_inventory(d);
            for(k = 0; k < 10; k++){
                if(d->PC->inventory[k] == NULL){
                    d->PC->inventory[k] = a;
                    break;
                }
            }
        }
        d->PC->speed += d->PC->equipment[5]->get_speed();
    }
    else if(d->PC->inventory[inventory_index]->get_type() == objtype_GLOVES){
        if(d->PC->equipment[6] == NULL){
            d->PC->equipment[6] = d->PC->inventory[inventory_index];
            d->PC->inventory[inventory_index] = NULL;
            shift_inventory(d);
        }
        else{
            object *a = d->PC->equipment[6];
            d->PC->equipment[6] = d->PC->inventory[inventory_index];
            d->PC->inventory[inventory_index] = NULL;
            shift_inventory(d);
            for(k = 0; k < 10; k++){
                if(d->PC->inventory[k] == NULL){
                    d->PC->inventory[k] = a;
                    break;
                }
            }
        }
        d->PC->speed += d->PC->equipment[6]->get_speed();
    }
    else if(d->PC->inventory[inventory_index]->get_type() == objtype_BOOTS){
        if(d->PC->equipment[7] == NULL){
            d->PC->equipment[7] = d->PC->inventory[inventory_index];
            d->PC->inventory[inventory_index] = NULL;
            shift_inventory(d);
        }
        else{
            object *a = d->PC->equipment[7];
            d->PC->equipment[7] = d->PC->inventory[inventory_index];
            d->PC->inventory[inventory_index] = NULL;
            shift_inventory(d);
            for(k = 0; k < 10; k++){
                if(d->PC->inventory[k] == NULL){
                    d->PC->inventory[k] = a;
                    break;
                }
            }
        }
        d->PC->speed += d->PC->equipment[7]->get_speed();
    }
    else if(d->PC->inventory[inventory_index]->get_type() == objtype_AMULET){
        if(d->PC->equipment[8] == NULL){
            d->PC->equipment[8] = d->PC->inventory[inventory_index];
            d->PC->inventory[inventory_index] = NULL;
            shift_inventory(d);
        }
        else{
            object *a = d->PC->equipment[8];
            d->PC->equipment[8] = d->PC->inventory[inventory_index];
            d->PC->inventory[inventory_index] = NULL;
            shift_inventory(d);
            for(k = 0; k < 10; k++){
                if(d->PC->inventory[k] == NULL){
                    d->PC->inventory[k] = a;
                    break;
                }
            }
        }
        d->PC->speed += d->PC->equipment[8]->get_speed();
    }
    else if(d->PC->inventory[inventory_index]->get_type() == objtype_LIGHT){
        if(d->PC->equipment[9] == NULL){
            d->PC->equipment[9] = d->PC->inventory[inventory_index];
            d->PC->inventory[inventory_index] = NULL;
            shift_inventory(d);
        }
        else{
            object *a = d->PC->equipment[9];
            d->PC->equipment[9] = d->PC->inventory[inventory_index];
            d->PC->inventory[inventory_index] = NULL;
            shift_inventory(d);
            for(k = 0; k < 10; k++){
                if(d->PC->inventory[k] == NULL){
                    d->PC->inventory[k] = a;
                    break;
                }
            }
        }
        d->PC->speed += d->PC->equipment[9]->get_speed();
    }
    else{
        if(d->PC->equipment[10] == NULL){
            d->PC->equipment[10] = d->PC->inventory[inventory_index];
            d->PC->inventory[inventory_index] = NULL;
            d->PC->speed += d->PC->equipment[10]->get_speed();
            shift_inventory(d);
        }
        else if(d->PC->equipment[11] == NULL){
            d->PC->equipment[11] = d->PC->inventory[inventory_index];
            d->PC->inventory[inventory_index] = NULL;
            d->PC->speed += d->PC->equipment[11]->get_speed();
            shift_inventory(d);
        }
        else{
            object *a = d->PC->equipment[11];
            d->PC->equipment[11] = d->PC->inventory[inventory_index];
            d->PC->inventory[inventory_index] = NULL;
            shift_inventory(d);
            for(k = 0; k < 10; k++){
                if(d->PC->inventory[k] == NULL){
                    d->PC->inventory[k] = a;
                    break;
                }
            }
            d->PC->speed += d->PC->equipment[11]->get_speed();
        }

    }

}

void display_description(dungeon_t *d, object *obj){
    //Clears screen for printing of inventory
    int i, j;
    for(i = 0; i < DUNGEON_Y + 2; i++){
        for(j = 0; j < DUNGEON_X + 2; j++){
            mvaddch(i, j, ' ');
        }
    }
    uint x = 10, y = 10, counter = 0, desc_index = 0;
    while(desc_index < strlen(obj->get_desc())){
        mvaddch(y, x, obj->get_desc()[desc_index]);
        desc_index++;
        x++;
        counter ++;
        if(counter > 39){
            counter = 0;
            y++;
            x = 10;
        }
    }
    getch();
}

void add_to_inventory(dungeon_t *d, int index){
    int i, placed = 0;
    for(i = 0; i < 10; i++){
        if(d->PC->inventory[i] == NULL){
            d->PC->inventory[i] = d->PC->equipment[index];
            d->PC->equipment[index] = NULL;
            placed = 1;
            break;
        }
    }
    if(placed == 0){
        io_queue_message("No free space in inventory, drop or expunge to unequip");
    }
}

void io_display_inventory(dungeon_t *d){
  //Clears screen for printing of inventory
  int i, j;
  for(i = 0; i < DUNGEON_Y + 2; i++){
    for(j = 0; j < DUNGEON_X + 2; j++){
      mvaddch(i, j, ' ');
    }
  }
  mvprintw(4, 15, "INVENTORY");

  if(d->PC->inventory[0] == NULL) {
    mvprintw(6, 15, "0)");
  }
  else{
    mvprintw(6, 15, "0)");
      mvprintw(6, 18, d->PC->inventory[0]->get_name());
  }

  if(d->PC->inventory[1] == NULL) {
    mvprintw(7, 15, "1)");
  }
  else{
    mvprintw(7, 15, "1)");
      mvprintw(7, 18, d->PC->inventory[1]->get_name());
  }

  if(d->PC->inventory[2] == NULL) {
    mvprintw(8, 15, "2)");
  }
  else{
    mvprintw(8, 15, "2)");
      mvprintw(8, 18, d->PC->inventory[2]->get_name());
  }

  if(d->PC->inventory[3] == NULL) {
    mvprintw(9, 15, "3)");
  }
  else{
    mvprintw(9, 15, "3)");
      mvprintw(9, 18, d->PC->inventory[3]->get_name());
  }

  if(d->PC->inventory[4] == NULL) {
    mvprintw(10, 15, "4)");
  }
  else{
    mvprintw(10, 15, "4)");
      mvprintw(10, 18, d->PC->inventory[4]->get_name());
  }

  if(d->PC->inventory[5] == NULL) {
    mvprintw(11, 15, "5)");
  }
  else{
    mvprintw(11, 15, "5)");
      mvprintw(11, 18, d->PC->inventory[5]->get_name());
  }

  if(d->PC->inventory[6] == NULL) {
    mvprintw(12, 15, "6)");
  }
  else{
    mvprintw(12, 15, "6)");
      mvprintw(12, 18, d->PC->inventory[6]->get_name());
  }

  if(d->PC->inventory[7] == NULL) {
    mvprintw(13, 15, "7)");
  }
  else{
    mvprintw(13, 15, "7)");
      mvprintw(13, 18, d->PC->inventory[7]->get_name());
  }

  if(d->PC->inventory[8] == NULL) {
    mvprintw(14, 15, "8)");
  }
  else{
    mvprintw(14, 15, "8)");
      mvprintw(14, 18, d->PC->inventory[8]->get_name());
  }

  if(d->PC->inventory[9] == NULL) {
    mvprintw(15, 15, "9)");
  }
  else{
    mvprintw(15, 15, "9)");
      mvprintw(15, 18, d->PC->inventory[9]->get_name());
  }

  mvprintw(19, 15, "Select [w]ear item, [d]rop item, e[x]punge item, [I]nspect item");
  mvprintw(20, 15, "OR press any other key to exit inventory");

  int key;
  switch (key = getch()) {
    case 'w':
        mvprintw(19, 15, "Choose an item to equip                                                ");
          mvprintw(20, 15, "                                                  ");
        switch (key = getch()) {
            case '0':
                if(d->PC->inventory[0] != NULL){
                    equip(d, 0);
                }
                break;
            case '1':
                if(d->PC->inventory[1] != NULL){
                    equip(d, 1);
                }
                break;
            case '2':
                if(d->PC->inventory[2] != NULL){
                    equip(d, 2);
                }
                break;
            case '3':
                if(d->PC->inventory[3] != NULL){
                    equip(d, 3);
                }
                break;
            case '4':
                if(d->PC->inventory[4] != NULL){
                    equip(d, 4);
                }
                break;
            case '5':
                if(d->PC->inventory[5] != NULL){
                    equip(d, 5);
                }
                break;
            case '6':
                if(d->PC->inventory[6] != NULL){
                    equip(d, 6);
                }
                break;
            case '7':
                if(d->PC->inventory[7] != NULL){
                    equip(d, 7);
                }
                break;
            case '8':
                if(d->PC->inventory[8] != NULL){
                    equip(d, 8);
                }
                break;
            case '9':
                if(d->PC->inventory[9] != NULL){
                    equip(d, 9);
                }
                break;
        }
      break;
    case 'd':
        mvprintw(19, 15, "Choose an item to drop                                                    ");
          mvprintw(20, 15, "                                                  ");
          switch (key = getch()) {
              case '0':
                  if(d->PC->inventory[0] != NULL){

                      d->objmap[d->PC->position[dim_y]][d->PC->position[dim_x]] = d->PC->inventory[0];
                      d->PC->inventory[0] = NULL;
                      shift_inventory(d);
                  }
                  break;
              case '1':
                  if(d->PC->inventory[1] != NULL){
                      d->objmap[d->PC->position[dim_y]][d->PC->position[dim_x]] = d->PC->inventory[1];
                      d->PC->inventory[1] = NULL;
                      shift_inventory(d);
                  }
                  break;
              case '2':
                  if(d->PC->inventory[2] != NULL){
                      d->objmap[d->PC->position[dim_y]][d->PC->position[dim_x]] = d->PC->inventory[2];
                      d->PC->inventory[2] = NULL;
                      shift_inventory(d);
                  }
                  break;
              case '3':
                  if(d->PC->inventory[3] != NULL){
                      d->objmap[d->PC->position[dim_y]][d->PC->position[dim_x]] = d->PC->inventory[3];
                      d->PC->inventory[3] = NULL;

                  }
                  break;
              case '4':
                  if(d->PC->inventory[4] != NULL){
                      d->objmap[d->PC->position[dim_y]][d->PC->position[dim_x]] = d->PC->inventory[4];
                      d->PC->inventory[4] = NULL;
                      shift_inventory(d);
                  }
                  break;
              case '5':
                  if(d->PC->inventory[5] != NULL){
                      d->objmap[d->PC->position[dim_y]][d->PC->position[dim_x]] = d->PC->inventory[5];
                      d->PC->inventory[5] = NULL;
                      shift_inventory(d);
                  }
                  break;
              case '6':
                  if(d->PC->inventory[6] != NULL){
                      d->objmap[d->PC->position[dim_y]][d->PC->position[dim_x]] = d->PC->inventory[6];
                      d->PC->inventory[6] = NULL;
                      shift_inventory(d);
                  }
                  break;
              case '7':
                  if(d->PC->inventory[7] != NULL){
                      d->objmap[d->PC->position[dim_y]][d->PC->position[dim_x]] = d->PC->inventory[7];
                      d->PC->inventory[7] = NULL;
                      shift_inventory(d);
                  }
                  break;
              case '8':
                  if(d->PC->inventory[8] != NULL){
                      d->objmap[d->PC->position[dim_y]][d->PC->position[dim_x]] = d->PC->inventory[8];
                      d->PC->inventory[8] = NULL;
                      shift_inventory(d);
                  }
                  break;
              case '9':
                  if(d->PC->inventory[9] != NULL){
                      d->objmap[d->PC->position[dim_y]][d->PC->position[dim_x]] = d->PC->inventory[9];
                      d->PC->inventory[9] = NULL;
                      shift_inventory(d);
                  }
                  break;
          }
          break;
    case 'x':
        mvprintw(19, 15, "Choose an item to expunge                                                ");
          mvprintw(20, 15, "                                                  ");
          switch (key = getch()) {
              case '0':
                  if(d->PC->inventory[0] != NULL){
                      delete(d->PC->inventory[0]);
                      d->PC->inventory[0] = NULL;
                      shift_inventory(d);
                  }
                  break;
              case '1':
                  if(d->PC->inventory[1] != NULL){
                      delete(d->PC->inventory[1]);
                      d->PC->inventory[1] = NULL;
                      shift_inventory(d);
                  }
                  break;
              case '2':
                  if(d->PC->inventory[2] != NULL){
                      delete(d->PC->inventory[2]);
                      d->PC->inventory[2] = NULL;
                      shift_inventory(d);
                  }
                  break;
              case '3':
                  if(d->PC->inventory[3] != NULL){
                      delete(d->PC->inventory[3]);
                      d->PC->inventory[3] = NULL;
                      shift_inventory(d);
                  }
                  break;
              case '4':
                  if(d->PC->inventory[4] != NULL){
                      delete(d->PC->inventory[4]);
                      d->PC->inventory[4] = NULL;
                      shift_inventory(d);
                  }
                  break;
              case '5':
                  if(d->PC->inventory[5] != NULL){
                      delete(d->PC->inventory[5]);
                      d->PC->inventory[5] = NULL;
                      shift_inventory(d);
                  }
                  break;
              case '6':
                  if(d->PC->inventory[6] != NULL){
                      delete(d->PC->inventory[6]);
                      d->PC->inventory[6] = NULL;
                      shift_inventory(d);
                  }
                  break;
              case '7':
                  if(d->PC->inventory[7] != NULL){
                      delete(d->PC->inventory[7]);
                      d->PC->inventory[7] = NULL;
                      shift_inventory(d);
                  }
                  break;
              case '8':
                  if(d->PC->inventory[8] != NULL){
                      delete(d->PC->inventory[8]);
                      d->PC->inventory[8] = NULL;
                      shift_inventory(d);
                  }
                  break;
              case '9':
                  if(d->PC->inventory[9] != NULL){
                      delete(d->PC->inventory[9]);
                      d->PC->inventory[9] = NULL;
                      shift_inventory(d);
                  }
                  break;
          }
      break;
    case 'I':
        mvprintw(19, 15, "Choose an item to inspect                                                ");
          mvprintw(20, 15, "                                                  ");
          switch (key = getch()) {
              case '0':
                  if(d->PC->inventory[0] != NULL){
                      display_description(d, d->PC->inventory[0]);
                  }
                  break;
              case '1':
                  if(d->PC->inventory[1] != NULL){
                      display_description(d, d->PC->inventory[1]);
                  }
                  break;
              case '2':
                  if(d->PC->inventory[2] != NULL){
                      display_description(d, d->PC->inventory[2]);
                  }
                  break;
              case '3':
                  if(d->PC->inventory[3] != NULL){
                      display_description(d, d->PC->inventory[3]);
                  }
                  break;
              case '4':
                  if(d->PC->inventory[4] != NULL){
                      display_description(d, d->PC->inventory[4]);
                  }
                  break;
              case '5':
                  if(d->PC->inventory[5] != NULL){
                      display_description(d, d->PC->inventory[5]);
                  }
                  break;
              case '6':
                  if(d->PC->inventory[6] != NULL){
                      display_description(d, d->PC->inventory[6]);
                  }
                  break;
              case '7':
                  if(d->PC->inventory[7] != NULL){
                      display_description(d, d->PC->inventory[7]);
                  }
                  break;
              case '8':
                  if(d->PC->inventory[8] != NULL){
                      display_description(d, d->PC->inventory[8]);
                  }
                  break;
              case '9':
                  if(d->PC->inventory[9] != NULL){
                      display_description(d, d->PC->inventory[9]);
                  }
                  break;
          }
      break;
    default:
      break;
  }
    for(i = 0; i < DUNGEON_Y + 2; i++){
        for(j = 0; j < DUNGEON_X + 2; j++){
            mvaddch(i, j, ' ');
        }
    }
}

void io_display_equipment(dungeon_t *d){
    //Clears screen for printing of inventory
    int i, j;
    for(i = 0; i < DUNGEON_Y + 2; i++){
        for(j = 0; j < DUNGEON_X + 2; j++){
            mvaddch(i, j, ' ');
        }
    }
    mvprintw(4, 15, "EQUIPMENT");

    if(d->PC->equipment[0] == NULL) {
        mvprintw(6, 15, "a)");
    }
    else{
        mvprintw(6, 15, "a)");
        mvprintw(6, 18, d->PC->equipment[0]->get_name());
    }

    if(d->PC->equipment[1] == NULL) {
        mvprintw(7, 15, "b)");
    }
    else{
        mvprintw(7, 15, "b)");
        mvprintw(7, 18, d->PC->equipment[1]->get_name());
    }

    if(d->PC->equipment[2] == NULL) {
        mvprintw(8, 15, "c)");
    }
    else{
        mvprintw(8, 15, "c)");
        mvprintw(8, 18, d->PC->equipment[2]->get_name());
    }

    if(d->PC->equipment[3] == NULL) {
        mvprintw(9, 15, "d)");
    }
    else{
        mvprintw(9, 15, "d)");
        mvprintw(9, 18, d->PC->equipment[3]->get_name());
    }

    if(d->PC->equipment[4] == NULL) {
        mvprintw(10, 15, "e)");
    }
    else{
        mvprintw(10, 15, "e)");
        mvprintw(10, 18, d->PC->equipment[4]->get_name());
    }

    if(d->PC->equipment[5] == NULL) {
        mvprintw(11, 15, "f)");
    }
    else{
        mvprintw(11, 15, "f)");
        mvprintw(11, 18, d->PC->equipment[5]->get_name());
    }

    if(d->PC->equipment[6] == NULL) {
        mvprintw(12, 15, "g)");
    }
    else{
        mvprintw(12, 15, "g)");
        mvprintw(12, 18, d->PC->equipment[6]->get_name());
    }

    if(d->PC->equipment[7] == NULL) {
        mvprintw(13, 15, "h)");
    }
    else{
        mvprintw(13, 15, "h)");
        mvprintw(13, 18, d->PC->equipment[7]->get_name());
    }

    if(d->PC->equipment[8] == NULL) {
        mvprintw(14, 15, "i)");
    }
    else{
        mvprintw(14, 15, "i)");
        mvprintw(14, 18, d->PC->equipment[8]->get_name());
    }

    if(d->PC->equipment[9] == NULL) {
        mvprintw(15, 15, "j)");
    }
    else{
        mvprintw(15, 15, "j)");
        mvprintw(15, 18, d->PC->equipment[9]->get_name());
    }

    if(d->PC->equipment[10] == NULL) {
        mvprintw(16, 15, "k)");
    }
    else{
        mvprintw(16, 15, "k)");
        mvprintw(16, 18, d->PC->equipment[10]->get_name());
    }

    if(d->PC->equipment[11] == NULL) {
        mvprintw(17, 15, "l)");
    }
    else{
        mvprintw(17, 15, "l)");
        mvprintw(17, 18, d->PC->equipment[11]->get_name());
    }

    mvprintw(20, 15, "Select [t]ake off an item");
    mvprintw(21, 15, "OR press any other key to exit equipment");

  int key;
  switch (key = getch()) {
    case 't':
        mvprintw(20, 15, "Choose an item to take off                                                ");
          mvprintw(21, 15, "                                                  ");
          switch (key = getch()) {
              case 'a':
                  if(d->PC->equipment[0] != NULL){
                      d->PC->speed -= d->PC->equipment[0]->get_speed();
                      add_to_inventory(d, 0);
                  }
                  break;
              case 'b':
                  if(d->PC->equipment[1] != NULL){
                      d->PC->speed -= d->PC->equipment[1]->get_speed();
                      add_to_inventory(d, 1);
                  }
                  break;
              case 'c':
                  if(d->PC->equipment[2] != NULL){
                      d->PC->speed -= d->PC->equipment[2]->get_speed();
                      add_to_inventory(d, 2);
                  }
                  break;
              case 'd':
                  if(d->PC->equipment[3] != NULL){
                      d->PC->speed -= d->PC->equipment[3]->get_speed();
                      add_to_inventory(d, 3);
                  }
                  break;
              case 'e':
                  if(d->PC->equipment[4] != NULL){
                      d->PC->speed -= d->PC->equipment[4]->get_speed();
                      add_to_inventory(d, 4);
                  }
                  break;
              case 'f':
                  if(d->PC->equipment[5] != NULL){
                      d->PC->speed -= d->PC->equipment[5]->get_speed();
                      add_to_inventory(d, 5);
                  }
                  break;
              case 'g':
                  if(d->PC->equipment[6] != NULL){
                      d->PC->speed -= d->PC->equipment[6]->get_speed();
                      add_to_inventory(d, 6);
                  }
                  break;
              case 'h':
                  if(d->PC->equipment[7] != NULL){
                      d->PC->speed -= d->PC->equipment[7]->get_speed();
                      add_to_inventory(d, 7);
                  }
                  break;
              case 'i':
                  if(d->PC->equipment[8] != NULL){
                      d->PC->speed -= d->PC->equipment[8]->get_speed();
                      add_to_inventory(d, 8);
                  }
                  break;
              case 'j':
                  if(d->PC->equipment[9] != NULL){
                      d->PC->speed -= d->PC->equipment[9]->get_speed();
                      add_to_inventory(d, 9);
                  }
                  break;
              case 'k':
                  if(d->PC->equipment[10] != NULL){
                      d->PC->speed -= d->PC->equipment[10]->get_speed();
                      add_to_inventory(d, 10);
                  }
                  break;
              case 'l':
                  if(d->PC->equipment[11] != NULL){
                      d->PC->speed -= d->PC->equipment[11]->get_speed();
                      add_to_inventory(d, 11);
                  }
                  break;
          }
          if(d->PC->speed < 1)
              d->PC->speed = 0;
      break;
    default:
      break;
  }
    for(i = 0; i < DUNGEON_Y + 2; i++){
        for(j = 0; j < DUNGEON_X + 2; j++){
            mvaddch(i, j, ' ');
        }
    }
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
      case 'i':
        io_display_inventory(d);
            fail_code = 1;
            break;
      case 'e':
        io_display_equipment(d);
            fail_code = 1;
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
