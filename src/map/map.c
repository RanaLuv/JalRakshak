
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "map.h"

// Check whether a node id is valid or not. 
static int valid_node(const Graph *g, int node)
{
    return g != NULL && node >= 0 && node < g->n;
}

//Creating an empty graph.
Graph *graph_create(int n)
{
    if (n <= 0 || n > MAX_NODES)
        return NULL;

    Graph *g = malloc(sizeof(Graph));

    if (g == NULL)
        return NULL;

    g->n = n;

    for (int i = 0; i < MAX_NODES; i++) {
        g->adj[i] = NULL;
        g->name[i][0] = '\0';
        g->is_shelter[i] = 0;
    }

    return g;
}

// Freeing every edge and then the graph.
void graph_free(Graph *g)
{
    if (g == NULL)
        return;

    for (int i = 0; i < g->n; i++) {
        Edge *current = g->adj[i];

        while (current != NULL) {
            Edge *temp = current;
            current = current->next;
            free(temp);
        }
    }

    free(g);
}

// Add an undirected road.
Status graph_add_edge(Graph *g, int u, int v, double w)
{
    if (!valid_node(g, u) || !valid_node(g, v) ||
        u == v || w < 0.0)
        return STATUS_INVALID;

    //Checking for an existing road.
    Edge *current = g->adj[u];

    while (current != NULL) {
        if (current->to == v)
            return STATUS_DUPLICATE;

        current = current->next;
    }

    // Allocating both directions before modifying the graph.
    Edge *uv = malloc(sizeof(Edge));
    Edge *vu = malloc(sizeof(Edge));

    if (uv == NULL || vu == NULL) {
        free(uv);
        free(vu);
        return STATUS_ERROR;
    }

    uv->to = v;
    uv->weight = w;
    uv->status = ROAD_OPEN;
    uv->next = g->adj[u];

    vu->to = u;
    vu->weight = w;
    vu->status = ROAD_OPEN;
    vu->next = g->adj[v];

    g->adj[u] = uv;
    g->adj[v] = vu;

    return STATUS_OK;
}

// Block or unblock a road in both directions.
Status graph_set_road(Graph *g, int u, int v, RoadStatus s)
{
    if (!valid_node(g, u) || !valid_node(g, v) ||
        u == v ||
        (s != ROAD_OPEN &&
         s != ROAD_BLOCKED &&
         s != ROAD_HAZARDOUS))
        return STATUS_INVALID;

    Edge *uv = g->adj[u];
    Edge *vu = g->adj[v];

    while (uv != NULL && uv->to != v)
        uv = uv->next;

    while (vu != NULL && vu->to != u)
        vu = vu->next;

    if (uv == NULL || vu == NULL)
        return STATUS_NOT_FOUND;

    uv->status = s;
    vu->status = s;

    return STATUS_OK;
}

// BFS: find all nodes reachable from src.
void graph_bfs(const Graph *g, int src, int visited[])
{
    if (visited == NULL)
        return;

    if (g == NULL) {
        return;
    }

    /* Initialize visited array */
    for (int i = 0; i < g->n; i++)
        visited[i] = 0;

    if (!valid_node(g, src))
        return;

    // Array-based queue.
    int queue[MAX_NODES];
    int front = 0;
    int rear = 0;

    visited[src] = 1;
    queue[rear++] = src;

    while (front < rear) {
        int u = queue[front++];
        Edge *current = g->adj[u];

        while (current != NULL) {
            int v = current->to;

            // Ignore blocked roads.
            if (current->status != ROAD_BLOCKED &&
                !visited[v]) {
                visited[v] = 1;
                queue[rear++] = v;
            }

            current = current->next;
        }
    }
}

// Return 1 if dst is reachable from src, otherwise 0.
int graph_reachable(const Graph *g, int src, int dst)
{
    if (!valid_node(g, src) || !valid_node(g, dst))
        return 0;

    int visited[MAX_NODES] = {0};

    graph_bfs(g, src, visited);

    return visited[dst];
}

// Find a location using its exact name.
int find_node(const Graph *g, const char *name)
{
    if (g == NULL || name == NULL)
        return -1;

    for (int i = 0; i < g->n; i++) {
        if (strcmp(g->name[i], name) == 0)
            return i;
    }

    return -1;
}

// Build a sample town with 9 locations and 2 shelters.
Graph *build_sample_map(void)
{
    Graph *g = graph_create(9);

    if (g == NULL)
        return NULL;

    const char *names[9] = {
        "Riverside",        
        "Market",          
        "OldTown",         
        "BridgeWest",      
        "BridgeEast",      
        "SchoolShelter",   
        "HillRoad",        
        "HillShelter",     
        "Hospital"         
    };

    for (int i = 0; i < 9; i++) {
        snprintf(g->name[i], MAX_NAME, "%s", names[i]);
    }

    g->is_shelter[5] = 1;
    g->is_shelter[7] = 1;

    // Roads on the Riverside side.
    if (graph_add_edge(g, 0, 1, 2.0) != STATUS_OK ||
        graph_add_edge(g, 0, 2, 3.0) != STATUS_OK ||
        graph_add_edge(g, 1, 2, 1.0) != STATUS_OK ||
        graph_add_edge(g, 2, 3, 2.0) != STATUS_OK) {
        graph_free(g);
        return NULL;
    }

    // The bridge connecting both sides.
    if (graph_add_edge(g, 3, 4, 1.0) != STATUS_OK) {
        graph_free(g);
        return NULL;
    }

    // Roads on the other side.
    if (graph_add_edge(g, 4, 5, 2.0) != STATUS_OK ||
        graph_add_edge(g, 4, 6, 2.0) != STATUS_OK ||
        graph_add_edge(g, 6, 7, 3.0) != STATUS_OK ||
        graph_add_edge(g, 6, 8, 1.0) != STATUS_OK ||
        graph_add_edge(g, 5, 8, 2.0) != STATUS_OK) {
        graph_free(g);
        return NULL;
    }

    return g;
}

// Display the adjacency list.
void graph_print(const Graph *g)
{
    if (g == NULL)
        return;

    printf("\nTown Map (Adjacency List)\n");

    for (int i = 0; i < g->n; i++) {
        printf("%d. %s%s -> ",
               i,
               g->name[i],
               g->is_shelter[i] ? " [SHELTER]" : "");

        Edge *current = g->adj[i];

        while (current != NULL) {
            const char *status;

            if (current->status == ROAD_OPEN)
                status = "OPEN";
            else if (current->status == ROAD_BLOCKED)
                status = "BLOCKED";
            else
                status = "HAZARDOUS";

            printf("%s(%.1f km, %s)",
                   g->name[current->to],
                   current->weight,
                   status);

            if (current->next != NULL)
                printf(" -> ");

            current = current->next;
        }

        printf("\n");
    }
}
