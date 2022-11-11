#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#define N 10000000
#define K 4

typedef struct Point
{
    float x, y;
} Point;

void init(Point *points, Point *clusters)
{
    srand(10);

    #pragma omp parallel for
    for (int i = 0; i < N; i++)
    {
        points[i].x = (float)rand() / RAND_MAX;
        points[i].y = (float)rand() / RAND_MAX;
    }

    #pragma omp parallel for
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

    #pragma omp parallel for
    for (int i = 0; i < N; i++)
    {
        // de onde vem este min??
        float min = 1000000000;
        int min_index = 0;
        float dist[K];

        // esta secção vai correr N*K vezes
        
        #pragma omp parallel for
        for (int j = 0; j < K; j++)
        {
            float distx = points[i].x - clusters[j].x;
            float disty = points[i].y - clusters[j].y;

            dist[j] = distx * distx + disty * disty;
        }

        for (int j = 0; j < K; j++)
        {
            if (dist[j] < min)
            {      
                min = dist[j];
            }
        }

        for (int j = 0; j < K; j++)
        {
            //Secção do código que garante resultados exatamente iguais aos do enunciado
            //if (dist[j] == min)
            //{   
            //    min_index = j;
            //    break;
            //}
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

int main(int argc, char** argv)
{
    int N = atoi(argv[1]);
    int K = atoi(argv[2]);
    int num_threads = atoi(argv[3]);

    Point *points  = malloc(N * sizeof(Point));
    Point clusters = malloc(K * sizeof(Point));
    int   count    = malloc(K * sizeof(Point));
    int iterator = 0;

    init(points, clusters);

    do
    {
        iterator++;
    } while (kmeans(points, clusters, count) && iterator < 20);

    printf("N = %d, K = %d\n ", N, K);
    for (int i = 0; i < K; i++)
    {
        printf("Center: (%f, %f) %d\n", clusters[i].x, clusters[i].y, count[i]);
    }
    printf("Iterations: %d times \n ", iterator);

    free(points);

    return 0;
}