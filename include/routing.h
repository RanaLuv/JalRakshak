
#ifndef ROUTING_H
#define ROUTING_H

#ifdef __cplusplus
extern "C" {
#endif

void dijkstra_test(double m[6][6], int src,
                   double dist[], int parent[]);

int build_path(const int parent[], int src,
               int dst, int path[]);

#ifdef __cplusplus
}
#endif

#endif
