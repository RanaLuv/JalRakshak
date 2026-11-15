
#ifndef MAP_H
#define MAP_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Edge {
    int to;
    double weight;
    RoadStatus status;
    struct Edge *next;
} Edge;

typedef struct {
    int n;
    Edge *adj[MAX_NODES];
    char name[MAX_NODES][MAX_NAME];
    int is_shelter[MAX_NODES];
} Graph;

Graph *graph_create(int n);
void graph_free(Graph *g);

Status graph_add_edge(Graph *g, int u, int v, double w);
Status graph_set_road(Graph *g, int u, int v, RoadStatus s);

void graph_bfs(const Graph *g, int src, int visited[]);
int graph_reachable(const Graph *g, int src, int dst);

Graph *build_sample_map(void);
int find_node(const Graph *g, const char *name);

void graph_print(const Graph *g);

#ifdef __cplusplus
}
#endif

#endif
