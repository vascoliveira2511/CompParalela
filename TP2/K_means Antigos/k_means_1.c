#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>




typedef struct Point
{
    float x, y;
} Point;



void init(Point *points, Point *clusters, const int N, const int K, const int n_threads)
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

int kmeans(Point *points, Point *clusters, int *count, const int N, const int K, const int n_threads)
{
    int changed = 0;
    
    float sum_dist_x[K];
    float sum_dist_y[K];

    for (int i = 0; i < K; i++)
    {
        count[i] = 0;
        sum_dist_x[i] = 0;
        sum_dist_y[i] = 0;
    }

    #pragma omp parallel num_threads(n_threads)
    {
        #pragma omp for firstprivate(clusters) reduction (+:sum_dist_x[:K]) reduction (+:sum_dist_y[:K]) reduction (+:count[:K]) 
        for (int i = 0; i < N; i++)
        {
            float dist [K];
            // de onde vem este min??
            float min = 10000;
            int min_index = 0;

            // esta secção vai correr N*K vezes
            // Este loop vai ser vetorizado e paralelizado. 
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
                min_index = dist[j] == min ? j : min_index;           
            }


            count[min_index]++;
            sum_dist_x[min_index] += points[i].x;
            sum_dist_y[min_index] += points[i].y;
            
        }
    }

    for (int i = 0; i < K; i++)
    {
        float x = sum_dist_x[i] / count[i];
        float y = sum_dist_y[i] / count[i];

        if (clusters[i].x != x || clusters[i].y != y)
        {
            clusters[i].x = x;
            clusters[i].y = y;
            changed = 1;
        }
    }

    return changed;
}

int main(int argc, char **argv)
{
    if (argc < 4 ) return -1;

    const int N = atoi(argv[1]);
    const int K = atoi(argv[2]);

    const int n_threads = atoi(argv[3]);

    //omp_set_num_threads(num_threads);

    Point *points = malloc(N * sizeof(Point));
    Point *clusters = malloc(K * sizeof(Point));
    int *count = malloc(K * sizeof(Point));
    int iterator = 0;

    init(points, clusters, N, K, n_threads);

    do
    {
        iterator++;
    } while (kmeans(points, clusters, count, N, K, n_threads) && iterator < 20);

    printf("N = %d, K = %d\n", N, K);
    for (int i = 0; i < K; i++)
    {
        printf("Center: (%.3f, %.3f) %d\n", clusters[i].x, clusters[i].y, count[i]);
    }
    printf("Iterations: %d times \n", iterator);

    free(points);
    free(clusters);
    free(count);

    return 0;
}