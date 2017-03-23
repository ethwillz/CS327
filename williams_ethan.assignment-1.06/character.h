#ifndef CHARACTER_H
# define CHARACTER_H

# include <stdint.h>

# include "dims.h"

typedef struct dungeon dungeon_t;
typedef struct npc npc_t;
typedef struct pc pc_t;
typedef struct dice_t dice_t;

typedef enum kill_type {
  kill_direct,
  kill_avenged,
  num_kill_types
} kill_type_t;

# ifdef __cplusplus

using namespace std;

class character{
 public:
  char symbol;
  pair_t position;
  int32_t speed;
  uint32_t alive;
  uint32_t sequence_number;
  npc_t *npc;
  pc_t *pc;
  uint32_t kills[num_kill_types];
  
};

extern "C" {
# else
typedef void character;
#endif

int32_t compare_characters_by_next_turn(const void *character1,
                                        const void *character2);
uint32_t can_see(dungeon_t *d, character *voyeur, character *exhibitionist);
void character_delete(void *c);

//Accesors and mutators for character class
uint32_t get_position_y(character *c);
void set_position_y(uint32_t newY, character *c);
uint32_t get_position_x(character *c);
void set_position_x(uint32_t newX, character *c);
uint32_t get_direct_kills(character *c);
  void set_direct_kills(character *c, int num);
uint32_t get_avenged_kills(character *c);
void set_avenged_kills(character *c, int num);
void delete_character(character *c);
uint8_t is_alive(character *c);
void set_alive(character *c, uint8_t isalive);
  char get_symbol(character *c);
void set_symbol(character *c, char desired);
  int32_t get_speed(character *c);
void set_speed(character *c, int32_t newspeed);
void set_sequence_num(character *c, uint32_t num);
  int16_t *get_position(character *c);

# ifdef __cplusplus
}
# endif

#endif
