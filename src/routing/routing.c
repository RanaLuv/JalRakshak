
#include <stdio.h>
#define N 6
#define INF 1000000000.0

#include "routing.h"

void dijkstra_test(double m[6][6], int src,
                   double dist[], int parent[])
{
    int visited[N] = {0};
    int i, j, u;
    double min;

    for (i = 0; i < N; i++)
    {
        dist[i] = INF;
        parent[i] = -1;
    }

    dist[src] = 0;

    for (i = 0; i < N; i++)
    {
        min = INF;
        u = -1;

        for (j = 0; j < N; j++)
        {
            if (!visited[j] && dist[j] < min)
            {
                min = dist[j];
                u = j;
            }
        }

        if (u == -1)
            break;

        visited[u] = 1;

        for (j = 0; j < N; j++)
        {
            if (m[u][j] > 0 && !visited[j])
            {
                if (dist[u] + m[u][j] < dist[j])
                {
                    dist[j] = dist[u] + m[u][j];
                    parent[j] = u;
                }
            }
        }
    }
}

int build_path(const int parent[], int src,
               int dst, int path[])
{
    int temp[N];
    int len = 0;
    int i;

    if (src < 0 || src >= N || dst < 0 || dst >= N)
        return -1;

    i = dst;

    while (i != -1)
    {
        temp[len++] = i;

        if (i == src)
            break;

        if (len >= N)
            return -1;

        i = parent[i];
    }

    if (temp[len - 1] != src)
        return -1;

    for (i = 0; i < len; i++)
    {
        path[i] = temp[len - 1 - i];
    }

    return len;
}
