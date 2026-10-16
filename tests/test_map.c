
#include <stdio.h>
#include "map.h"

int main(void)
{
    Graph *g = build_sample_map();

    if (g == NULL) {
        printf("Failed to create sample map.\n");
        return 1;
    }

    /* Find locations by name */
    int home = find_node(g, "Riverside");
    int school = find_node(g, "SchoolShelter");
    int hill = find_node(g, "HillShelter");

    if (home == -1 || school == -1 || hill == -1) {
        printf("Location lookup failed.\n");
        graph_free(g);
        return 1;
    }

    /* Display the complete map */
    graph_print(g);

    /* Test connectivity before blocking the bridge */
    printf("\n--- Before Blocking Bridge ---\n");

    printf("Riverside -> SchoolShelter: %s\n",
           graph_reachable(g, home, school)
               ? "Reachable" : "Not reachable");

    printf("Riverside -> HillShelter: %s\n",
           graph_reachable(g, home, hill)
               ? "Reachable" : "Not reachable");

    /* Block the bridge between nodes 3 and 4 */
    printf("\n--- Blocking Bridge ---\n");

    Status result = graph_set_road(
        g, 3, 4, ROAD_BLOCKED
    );

    if (result != STATUS_OK) {
        printf("Failed to block bridge.\n");
        graph_free(g);
        return 1;
    }

    printf("Bridge blocked successfully.\n");

    /* Check connectivity again */
    printf("\n--- After Blocking Bridge ---\n");

    printf("Riverside -> SchoolShelter: %s\n",
           graph_reachable(g, home, school)
               ? "Reachable" : "Not reachable");

    printf("Riverside -> HillShelter: %s\n",
           graph_reachable(g, home, hill)
               ? "Reachable" : "Not reachable");

    /* Demonstrate BFS */
    int visited[MAX_NODES] = {0};
    graph_bfs(g, home, visited);

    printf("\nLocations reachable from Riverside:\n");

    for (int i = 0; i < g->n; i++) {
        if (visited[i])
            printf("- %s\n", g->name[i]);
    }

    /* Unblock the bridge and test again */
    graph_set_road(g, 3, 4, ROAD_OPEN);

    printf("\nAfter reopening bridge:\n");
    printf("Riverside -> SchoolShelter: %s\n",
           graph_reachable(g, home, school)
               ? "Reachable" : "Not reachable");

    /* Release allocated memory */
    graph_free(g);

    return 0;
}
