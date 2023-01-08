#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

typedef struct Point
{
    float x, y;
} Point;

void init(Point *points, Point *clusters, const int N, const int K)
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

int kmeans(Point *points, Point *clusters, int *count, const int start, const int end, const int K)
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

    for (int i = start; i < end; i++)
    {
        float dist[K];
        float min_value = 10000;
        int min_index = 0;

        for (int j = 0; j < K; j++)
        {
            float distx = points[i].x - clusters[j].x;
            float disty = points[i].y - clusters[j].y;

            dist[j] = distx * distx + disty * disty;
        }

        for (int j = 0; j < K; j++)
        {
            if (dist[j] < min_value)
            {
                min_value = dist[j];
            }
        }

        for (int j = 0; j < K; j++)
            min_index = dist[j] == min_value ? j : min_index;

        count[min_index]++;
        sum_dist_x[min_index] += points[i].x;
        sum_dist_y[min_index] += points[i].y;
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
    if (argc < 4)
        return -1;

    const int N = atoi(argv[1]);
    const int K = atoi(argv[2]);
    int num_procs, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    Point *points = malloc(N * sizeof(Point));
    Point *clusters = malloc(K * sizeof(Point));
    int *count = malloc(K * sizeof(Point));
    int iterator = 0;

    if (rank == 0)
        init(points, clusters, N, K);

    int chunk_size = N / num_procs;
    int start = rank * chunk_size;
    int end = start + chunk_size;

    Point *local_points = malloc(chunk_size * sizeof(Point));
    for (int i = start; i < end; i++)
    {
        local_points[i - start] = points[i];
    }

    int local_iterator = 0;
    int local_changed;
    do
    {
        local_iterator++;
        local_changed = kmeans(local_points, clusters, count, start, end, K);
    } while (local_changed && local_iterator < 20);

    int global_changed;
    MPI_Allreduce(&local_changed, &global_changed, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    if (!global_changed)
    {
        // k-means algorithm has converged
        break;
    }

    int global_count[K];
    MPI_Reduce(count, global_count, K, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    float global_sum_dist_x[K];
    float global_sum_dist_y[K];
    MPI_Reduce(sum_dist_x, global_sum_dist_x, K, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(sum_dist_y, global_sum_dist_y, K, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        for (int i = 0; i < K; i++)
        {
            clusters[i].x = global_sum_dist_x[i] / global_count[i];
            clusters[i].y = global_sum_dist_y[i] / global_count[i];
        }
    }

    int global_iterator;
    MPI_Reduce(&local_iterator, &global_iterator, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        printf("N = %d, K = %d\n", N, K);
        for (int i = 0; i < K; i++)
        {
            printf("Center: (%.3f, %.3f) %d\n", clusters[i].x, clusters[i].y, global_count[i]);
        }
        printf("Iterations: %d times \n", global_iterator);
    }
    free(local_points);
    free(points);
    free(clusters);
    free(count);

    MPI_Finalize();

    return 0;
}
