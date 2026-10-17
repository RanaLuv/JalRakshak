
#include <iostream>
#include <cassert>
#include "map.h"

int main() {
    std::cout << "=== JalRakshak Map Module Tests ===\n\n";

    // TEST 1: Create a graph
    Graph* g = graph_create(4);
    assert(g != nullptr);
    std::cout << "Test 1 passed: Graph created.\n";

    // TEST 2: Add roads
    assert(graph_add_edge(g, 0, 1, 2.0) == STATUS_OK);
    assert(graph_add_edge(g, 1, 2, 3.0) == STATUS_OK);
    assert(graph_add_edge(g, 2, 3, 1.0) == STATUS_OK);
    std::cout << "Test 2 passed: Roads added.\n";

    // TEST 3: Check connectivity
    assert(graph_reachable(g, 0, 3) == 1);
    assert(graph_reachable(g, 0, 2) == 1);
    assert(graph_reachable(g, 0, 0) == 1);
    std::cout << "Test 3 passed: Reachability works.\n";

    // TEST 4: Block a road
    assert(graph_set_road(g, 1, 2, ROAD_BLOCKED) == STATUS_OK);
    std::cout << "Test 4 passed: Road status updated.\n";

    // TEST 5: Test BFS
    int visited[MAX_NODES] = {0};
    graph_bfs(g, 0, visited);
    std::cout << "BFS visited nodes: ";

    for (int i = 0; i < g->n; i++) {
        if (visited[i]) {
            std::cout << i << " ";
        }
    }
    std::cout << "\n";

    // TEST 6: Display graph
    std::cout << "\nMap details:\n";
    graph_print(g);

    // TEST 7: Free memory
    graph_free(g);
    std::cout << "\nTest 7 passed: Graph memory freed.\n";

    std::cout << "\nAll map tests completed.\n";
    return 0;
}
