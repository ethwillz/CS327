#ifndef OBJECT_H
#define OBJECT_H

# include <stdint.h>
#include <vector>

# include "dims.h"
# include "character.h"
#include "dice.h"
#include "descriptions.h"
#include "dungeon.h"
#include "object.h"
#include "utils.h"
#include "npc.h"

using namespace std;

class object{
public:
    string name;
    string description;
    object_type_t type;
    uint32_t color;
    //Dice values
    int32_t hit;
    dice damage;
    int32_t dodge;
    int32_t defence;
    int32_t weight;
    int32_t speed;
    int32_t attribute;
    int32_t value;
    pair_t position;
};

//typedef struct dungeon dungeon_t;

void generate_objects(dungeon *d, uint16_t num_objects);

#endif //OBJECT_H
