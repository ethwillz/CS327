//
// Created by ethwillz on 3/31/17.
//

#include <cstring>
#include "object.h"
#include "utils.h"
#include "npc.h"
#include "dungeon.h"

object generate_object(dungeon *d) {
    uint32_t i;
    object *m;
    uint32_t room;
    pair_t p;
    uint32_t c;

    for (i = 0; i < d->object_descriptions.size(); i++) {
        m = new object;
        memset(m, 0, sizeof(*m));

        do {
            room = rand_range(1, d->num_rooms - 1);
            p[dim_y] = rand_range(d->rooms[room].position[dim_y],
                                  (d->rooms[room].position[dim_y] +
                                   d->rooms[room].size[dim_y] - 1));
            p[dim_x] = rand_range(d->rooms[room].position[dim_x],
                                  (d->rooms[room].position[dim_x] +
                                   d->rooms[room].size[dim_x] - 1));
        } while (d->character_map[p[dim_y]][p[dim_x]]);
        m->position[dim_y] = p[dim_y];
        m->position[dim_x] = p[dim_x];
        //TODO figure out how to map objects
        d->character_map[p[dim_y]][p[dim_x]] = m;
        //m->alive = 1;
        //m->sequence_number = ++d->character_sequence_number;
        //m->characteristics = rand() & 0x0000000f;

        m->name = d->object_descriptions[i].get_name();
        m->description = d->object_descriptions[i].get_description();
        m->type = d->object_descriptions[i].get_type();
        m->color = d->object_descriptions[i].get_color();
        m->hit = d->object_descriptions[i].get_hit().roll();
        m->damage = d->object_descriptions[i].get_damage();
        m->dodge = d->object_descriptions[i].get_dodge().roll();
        m->defence = d->object_descriptions[i].get_defence().roll();
        m->weight = d->object_descriptions[i].get_weight().roll();
        m->speed = d->object_descriptions[i].get_speed().roll();
        m->attribute = d->object_descriptions[i].get_attribute().roll();
        m->value= d->object_descriptions[i].get_value().roll();
    }
}