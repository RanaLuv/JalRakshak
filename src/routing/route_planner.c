#include <stdio.h>
#include "common.h"
#include "map.h"
#include "routing.h"

#define FAR_AWAY 1000000.0

Route find_best_route(const Graph *g, int src, int mode) {
    double dist[MAX_NODES];
    int parent[MAX_NODES];
    int done[MAX_NODES];
    int temp[MAX_NODES];
    int i, u, steps, best, node, count;
    double cost;
    Edge *e;
    Route r;

    r.length = 0;
    r.total_distance = 0;
    r.status = STATUS_NOT_FOUND;
    if (src < 0 || src >= g->n) {
        return r;
    }

    for (i = 0; i < g->n; i++) {
        dist[i] = FAR_AWAY;
        parent[i] = -1;
        done[i] = 0;
    }
    dist[src] = 0;

    for (steps = 0; steps < g->n; steps++) {
        u = -1;
        for (i = 0; i < g->n; i++) {
            if (done[i] == 0 && dist[i] < FAR_AWAY) {
                if (u == -1 || dist[i] < dist[u]) {
                    u = i;
                }
            }
        }
        if (u == -1) {
            break;
        }
        done[u] = 1;
        e = g->adj[u];
        while (e != NULL) {
            if (e->status != ROAD_BLOCKED) {
                cost = e->weight;
                if (e->status == ROAD_HAZARDOUS) {
                    if (mode == 1) {
                        cost = cost * 5;
                    } else {
                        cost = cost * 1.5;
                    }
                }
                if (dist[u] + cost < dist[e->to]) {
                    dist[e->to] = dist[u] + cost;
                    parent[e->to] = u;
                }
            }
            e = e->next;
        }
    }

    best = -1;
    for (i = 0; i < g->n; i++) {
        if (g->is_shelter[i] == 1 && dist[i] < FAR_AWAY) {
            if (best == -1 || dist[i] < dist[best]) {
                best = i;
            }
        }
    }
    if (best == -1) {
        return r;
    }

    count = 0;
    node = best;
    while (node != -1) {
        temp[count] = node;
        count++;
        node = parent[node];
    }
    for (i = 0; i < count; i++) {
        r.path[i] = temp[count - 1 - i];
    }
    r.length = count;
    r.total_distance = dist[best];
    r.status = STATUS_OK;
    return r;
}
