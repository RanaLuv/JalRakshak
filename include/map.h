
#ifndef MAP_H
#define MAP_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* A road connecting one node to another */
typedef struct Edge {
    int to;
    double weight;
    RoadStatus status;
    struct Edge *next;
} Edge;

/* The complete town map */
typedef struct {
    int n;
    Edge *adj[MAX_NODES];
    char name[MAX_NODES][MAX_NAME];
    int is_shelter[MAX_NODES];
} Graph;

/* Graph creation and memory management */
Graph *graph_create(int n);
void graph_free(Graph *g);

/* Road management */
Status graph_add_edge(Graph *g, int u, int v, double w);
Status graph_set_road(Graph *g, int u, int v, RoadStatus s);

/* Connectivity */
void graph_bfs(const Graph *g, int src, int visited[]);
int graph_reachable(const Graph *g, int src, int dst);

/* Sample map and location lookup */
Graph *build_sample_map(void);
int find_node(const Graph *g, const char *name);

/* Optional helper for displaying the map */
void graph_print(const Graph *g);

#ifdef __cplusplus
}
#endif

#endif
