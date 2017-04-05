//
// Created by ethwillz on 3/31/17.
//
#include <stdlib.h>

#include <cstring>
#include "dungeon.h"

void generate_objects(dungeon *d, uint16_t num_objects) {
    uint32_t i;
    object *m;
    uint32_t room;
    pair_t p;
    uint64_t selected;

    for (i = 0; i < num_objects; i++) {
        m = new object;
        selected = rand_range(0, (d->object_descriptions).size()-1);
        memset(m, 0, sizeof(*m));

        do {
            room = rand_range(1, d->num_rooms - 1);
            p[dim_y] = rand_range(d->rooms[room].position[dim_y],
                                  (d->rooms[room].position[dim_y] +
                                   d->rooms[room].size[dim_y] - 1));
            p[dim_x] = rand_range(d->rooms[room].position[dim_x],
                                  (d->rooms[room].position[dim_x] +
                                   d->rooms[room].size[dim_x] - 1));
        } while (d->object_map[p[dim_y]][p[dim_x]]);
        m->position[dim_y] = p[dim_y];
        m->position[dim_x] = p[dim_x];
        d->object_map[p[dim_y]][p[dim_x]] = m;

        m->name = d->object_descriptions[selected].get_name();
        m->description = d->object_descriptions[selected].get_description();
        m->type = d->object_descriptions[selected].get_type();
        m->color = d->object_descriptions[selected].get_color();
        m->hit = d->object_descriptions[selected].get_hit().roll();
        m->damage = d->object_descriptions[selected].get_damage();
        m->dodge = d->object_descriptions[selected].get_dodge().roll();
        m->defence = d->object_descriptions[selected].get_defence().roll();
        m->weight = d->object_descriptions[selected].get_weight().roll();
        m->speed = d->object_descriptions[selected].get_speed().roll();
        m->attribute = d->object_descriptions[selected].get_attribute().roll();
        m->value= d->object_descriptions[selected].get_value().roll();

        d->object_map[p[dim_y]][p[dim_x]] = m;
    }
}
