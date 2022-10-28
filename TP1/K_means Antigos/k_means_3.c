#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 10000000
#define K 4

typedef struct Point
{
    float x, y;
} Point;

void init(Point *points, Point *clusters)
{
    srand(10);
    for (int i = 0; i < N; i++)
    {
        points[i].x = (float)rand() / RAND_MAX;
        points[i].y = (float)rand() / RAND_MAX;
    }

    for (int i = 0; i < K; i++)
    {
        clusters[i].x = points[i].x;
        clusters[i].y = points[i].y;
    }
}

int kmeans(Point *points, Point *clusters, int *count)
{
    int changed = 0;
    Point sum_of_distances[K];

    for (int i = 0; i < K; i++)
    {
        count[i] = 0;
        sum_of_distances[i].x = 0;
        sum_of_distances[i].y = 0;
    }

    for (int i = 0; i < N; i++)
    {
        // de onde vem este min??
        float min = 1000000000;
        int min_index = 0;
        float dist[K];

        // esta secção vai correr N*K vezes
        for (int j = 0; j < K; j++)
        {
            float distx = points[i].x - clusters[j].x;
            float disty = points[i].y - clusters[j].y;

            dist[j] = distx * distx + disty * disty;
        }

        for (int j = 0; j < K; j++)
        {

             min = dist[j] < min ? dist[j] : min;
        }

        for (int j = 0; j < K; j++)
        {
             min_index = dist[j] == min ? j : min_index;
        }
        


        count[min_index]++;
        sum_of_distances[min_index].x += points[i].x;
        sum_of_distances[min_index].y += points[i].y;
    }

    for (int i = 0; i < K; i++)
    {
        float x = sum_of_distances[i].x / count[i];
        float y = sum_of_distances[i].y / count[i];

        if (clusters[i].x != x || clusters[i].y != y)
        {
            clusters[i].x = x;
            clusters[i].y = y;
            changed = 1;
        }
    }

    return changed;
}

int main()
{
    Point *points = malloc(N * sizeof(Point));
    // float *py = (float *)malloc(N * sizeof(float));
    Point clusters[K];
    // float *cy = (float *)malloc(K * sizeof(float));

    int iterator = 0;
    int count[K];

    init(points, clusters);

    do
    {
        iterator++;
    } while (kmeans(points, clusters, count));

    printf("Iterations: %d times \n ", iterator);
    for (int i = 0; i < K; i++)
    {
        printf("Cluster %d: (%f, %f) \n", i, clusters[i].x, clusters[i].y);
        printf("Count: %d \n", count[i]);
    }

    free(points);

    return 0;
}