#ifndef PATH_H
# define PATH_H

typedef struct dungeon dungeon_t;

# ifdef __cplusplus
extern "C" {
# endif

void dijkstra(dungeon_t *d);
void dijkstra_tunnel(dungeon_t *d);

# ifdef __cplusplus
}
# endif
#endif
