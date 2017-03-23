#ifndef PC_H
# define PC_H

# include <stdint.h>

# include "dims.h"

#define DUNGEON_X              160
#define DUNGEON_Y              105

typedef struct dungeon dungeon_t;

#ifdef __cplusplus

using namespace std;

class pc: public character{
 public:
  uint8_t pc_visible[DUNGEON_Y][DUNGEON_X];
};

extern "C" {
# else
typedef void pc;
#endif

void pc_delete(pc *pc);
uint32_t pc_is_alive(dungeon_t *d);
void config_pc(dungeon_t *d);
uint32_t pc_next_pos(dungeon_t *d, pair_t dir);
void place_pc(dungeon_t *d);
uint32_t pc_in_room(dungeon_t *d, uint32_t room);

  uint8_t is_visible(pc *c, int y, int x);
  void set_visibility(pc *c, int y, int x, int visible);

#ifdef __cplusplus
}
#endif

#endif
