#include "move.h"

#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#include "dungeon.h"
#include "heap.h"
#include "move.h"
#include "npc.h"
#include "pc.h"
#include "character.h"
#include "utils.h"
#include "path.h"
#include "event.h"
#include "io.h"
#include "object.h"

void do_combat(dungeon_t *d, character *atk, character *def)
{
  uint32_t damage, i;

  if (character_is_alive(def)) {
    if (atk != d->PC) {
      damage = atk->damage->roll();
      io_queue_message("The %s hits you for %d.", atk->name, damage);
    } else {
      for (i = damage = 0; i < num_eq_slots; i++) {
        if (i == eq_slot_weapon && !d->PC->eq[i]) {
          damage += atk->damage->roll();
        } else if (d->PC->eq[i] && i != 2) {
          damage += d->PC->eq[i]->roll_dice();
        }
      }
      io_queue_message("You hit the %s for %d.", def->name, damage);
    }

    if (damage >= def->hp) {
      if (atk != d->PC) {
        io_queue_message("You die.");
        io_queue_message(""); /* Extra message to force pause on "more" */
      } else {
        io_queue_message("The %s dies.", def->name);
      }
      def->hp = 0;
      def->alive = 0;
      character_increment_dkills(atk);
      character_increment_ikills(atk, (character_get_dkills(def) +
                                       character_get_ikills(def)));
      if (def != d->PC) {
        d->num_monsters--;
      }
      charpair(def->position) = NULL;
    } else {
      def->hp -= damage;
    }

    if (def != d->PC) {
      d->num_monsters--;
    }
  }
}

void move_character(dungeon_t *d, character *c, pair_t next)
{
    pair_t displacement;
  uint32_t found_cell;
  pair_t order[9] = {
    { -1, -1 },
    { -1,  0 },
    { -1,  1 },
    {  0, -1 },
    {  0,  0 },
    {  0,  1 },
    {  1, -1 },
    {  1,  0 },
    {  1,  1 },
  };
  uint32_t s, i;

  if (charpair(next) &&
      ((next[dim_y] != c->position[dim_y]) ||
       (next[dim_x] != c->position[dim_x]))) {
    if ((charpair(next) == d->PC) ||
        c == d->PC) {
      do_combat(d, c, charpair(next));
    } else {
      /* Easiest way for a monster to displace another monster is *
       * to swap them.  This could lead to some strangeness where *
       * two monsters of the exact same speed continually         *
       * displace each other and never make progress, but I don't *
       * have any real problem with that.  When we have better    *
       * game balance, weaker monsters should not be able to      *
       * displace stronger monsters.                              */
      /* Turns out I don't like swapping them after all.  We'll   *
       * instead select a random square from the 8 surrounding    *
       * the target cell.  Keep doing it until either we swap or  *
       * find an empty one for the displacement.                  */
      for (s = rand() % 9, found_cell = i = 0;
           i < 9 && !found_cell; i++) {
        displacement[dim_y] = next[dim_y] + order[s % 9][dim_y];
        displacement[dim_x] = next[dim_x] + order[s % 9][dim_x];
        if (((npc *) charpair(next))->characteristics & NPC_PASS_WALL) {
          if (!charpair(displacement) ||
              (charpair(displacement) == c)) {
            found_cell = 1;
          }
        } else {
          if ((!charpair(displacement) &&
               (mappair(displacement) >= ter_floor)) ||
              (charpair(displacement) == c)) {
            found_cell = 1;
          }
        }
      }

      if (!found_cell) {
        return;
      }

      assert(charpair(next));

      charpair(c->position) = NULL;
      charpair(displacement) = charpair(next);
      charpair(next) = c;
      charpair(displacement)->position[dim_y] = displacement[dim_y];
      charpair(displacement)->position[dim_x] = displacement[dim_x];
      c->position[dim_y] = next[dim_y];
      c->position[dim_x] = next[dim_x];
    }
  } else {
    /* No character in new position. */

    d->character_map[c->position[dim_y]][c->position[dim_x]] = NULL;
    c->position[dim_y] = next[dim_y];
    c->position[dim_x] = next[dim_x];
    d->character_map[c->position[dim_y]][c->position[dim_x]] = c;

      if(d->map[c->position[dim_y]][c->position[dim_x]] == ter_water && !c->in_water){
          if(c == d->PC) {
              io_queue_message("You have entered the water, cutting your speed in half!");
          }
          c->speed = c->speed / 2;
          c->in_water = 1;
      }

      if(d->map[c->position[dim_y]][c->position[dim_x]] != ter_water && c->in_water){
          c->speed = c->speed * 2;
          c->in_water = 0;
      }

      if(d->map[c->position[dim_y]][c->position[dim_x]] == ter_lava){
          c->hp = c->hp - 50;
          if(c == d->PC) {
              io_queue_message("You are in lava and have lost 50hp!");
          }
      }
  }

  if (c == d->PC) {
    pc_reset_visibility((pc *) c);
    pc_observe_terrain((pc *) c, d);
  }
}

void do_moves(dungeon_t *d)
{
  pair_t next;
  character *c;
  event_t *e;

  /* Remove the PC when it is PC turn.  Replace on next call.  This allows *
   * use to completely uninit the heap when generating a new level without *
   * worrying about deleting the PC.                                       */

  if (pc_is_alive(d)) {
    /* The PC always goes first one a tie, so we don't use new_event().  *
     * We generate one manually so that we can set the PC sequence       *
     * number to zero.                                                   */
    e = (event_t *) malloc(sizeof (*e));
    e->type = event_character_turn;
    /* The next line is buggy.  Monsters get first turn before PC.  *
     * Monster gen code always leaves PC in a monster-free room, so *
     * not a big issue, but it needs a better solution.             */
    e->time = d->time + (1000 / character_get_speed(d->PC));
    e->sequence = 0;
    e->c = d->PC;
    heap_insert(&d->events, e);
  }

  while (pc_is_alive(d) &&
         (e = (event_t *) heap_remove_min(&d->events)) &&
         ((e->type != event_character_turn) || (e->c != d->PC))) {
    d->time = e->time;
    if (e->type == event_character_turn) {
      c = e->c;
    }
    if (!character_is_alive(c)) {
      if (d->character_map[character_get_y(c)][character_get_x(c)] == c) {
        d->character_map[character_get_y(c)][character_get_x(c)] = NULL;
      }
      if (c != d->PC) {
        event_delete(e);
      }
      continue;
    }

    npc_next_pos(d, (npc *) c, next);
    move_character(d, c, next);

    heap_insert(&d->events, update_event(d, e, 1000 / character_get_speed(c)));
  }

  io_display(d);
  if (pc_is_alive(d) && e->c == d->PC) {
    c = e->c;
    d->time = e->time;
    /* Kind of kludgey, but because the PC is never in the queue when   *
     * we are outside of this function, the PC event has to get deleted *
     * and recreated every time we leave and re-enter this function.    */
    e->c = NULL;
    event_delete(e);
    io_handle_input(d);
  }
}

void dir_nearest_wall(dungeon_t *d, character *c, pair_t dir)
{
  dir[dim_x] = dir[dim_y] = 0;

  if (character_get_x(c) != 1 && character_get_x(c) != DUNGEON_X - 2) {
    dir[dim_x] = (character_get_x(c) > DUNGEON_X - character_get_x(c) ? 1 : -1);
  }
  if (character_get_y(c) != 1 && character_get_y(c) != DUNGEON_Y - 2) {
    dir[dim_y] = (character_get_y(c) > DUNGEON_Y - character_get_y(c) ? 1 : -1);
  }
}

uint32_t against_wall(dungeon_t *d, character *c)
{
  return ((mapxy(character_get_x(c) - 1,
                 character_get_y(c)    ) == ter_wall_immutable) ||
          (mapxy(character_get_x(c) + 1,
                 character_get_y(c)    ) == ter_wall_immutable) ||
          (mapxy(character_get_x(c)    ,
                 character_get_y(c) - 1) == ter_wall_immutable) ||
          (mapxy(character_get_x(c)    ,
                 character_get_y(c) + 1) == ter_wall_immutable));
}

uint32_t in_corner(dungeon_t *d, character *c)
{
  uint32_t num_immutable;

  num_immutable = 0;

  num_immutable += (mapxy(character_get_x(c) - 1,
                          character_get_y(c)    ) == ter_wall_immutable);
  num_immutable += (mapxy(character_get_x(c) + 1,
                          character_get_y(c)    ) == ter_wall_immutable);
  num_immutable += (mapxy(character_get_x(c)    ,
                          character_get_y(c) - 1) == ter_wall_immutable);
  num_immutable += (mapxy(character_get_x(c)    ,
                          character_get_y(c) + 1) == ter_wall_immutable);

  return num_immutable > 1;
}


static void new_dungeon_level(dungeon_t *d, uint32_t dir)
{
  /* Eventually up and down will be independantly meaningful. *
   * For now, simply generate a new dungeon.                  */

  switch (dir) {
  case '<':
  case '>':
    new_dungeon(d);
    break;
  default:
    break;
  }
}


uint32_t move_pc(dungeon_t *d, uint32_t dir)
{
  pair_t next;
  uint32_t was_stairs = 0;

  next[dim_y] = character_get_y(d->PC);
  next[dim_x] = character_get_x(d->PC);


  switch (dir) {
  case 1:
  case 2:
  case 3:
    next[dim_y]++;
    break;
  case 4:
  case 5:
  case 6:
    break;
  case 7:
  case 8:
  case 9:
    next[dim_y]--;
    break;
  }
  switch (dir) {
  case 1:
  case 4:
  case 7:
    next[dim_x]--;
    break;
  case 2:
  case 5:
  case 8:
    break;
  case 3:
  case 6:
  case 9:
    next[dim_x]++;
    break;
  case '<':
    if (mappair(character_get_pos(d->PC)) == ter_stairs_up) {
      was_stairs = 1;
      new_dungeon_level(d, '<');
    }
    break;
  case '>':
    if (mappair(character_get_pos(d->PC)) == ter_stairs_down) {
      was_stairs = 1;
      new_dungeon_level(d, '>');
    }
    break;
  }

  if (was_stairs) {
    return 0;
  }

  if ((dir != '>') && (dir != '<') && (mappair(next) >= ter_floor)) {
    move_character(d, d->PC, next);
    io_update_offset(d);
    dijkstra(d);
    dijkstra_tunnel(d);
    d->PC->pick_up(d);

    return 0;
  }

  return 1;
}
