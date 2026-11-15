
#include <iostream>
#include "routing.h"

using namespace std;

int main()
{
    double m[6][6] = {0};

    m[0][1] = m[1][0] = 4;
    m[1][2] = m[2][1] = 3;
    m[0][3] = m[3][0] = 10;
    m[2][3] = m[3][2] = 2;

    double dist[6];
    int parent[6];
    int path[6];

    dijkstra_test(m, 0, dist, parent);

    int len = build_path(parent, 0, 3, path);

    if (len == -1)
    {
        cout << "No route available" << endl;
    }
    else
    {
        cout << "Shortest distance: " << dist[3] << endl;

        cout << "Path: ";

        for (int i = 0; i < len; i++)
        {
            cout << path[i] << " ";
        }

        cout << endl;
    }

    return 0;
}
