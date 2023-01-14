/*******************************************************************************
 * Regras: - Só o rank 0 é que muda as coordenadas de clusters
 *         - Todos os ranks terão sums e counts que terão dps de enviar para 0
 *         - 0 receberá os sums e os counts


*/

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

int kmeans(Point *points, Point *clusters, float *sumx, float *sumy, int *count, const int N, const int K)
{
    int changed = 0;

    for (int i = 0; i < K; i++)
    {
        count[i] = 0;
        sumx[i] = 0;
        sumy[i] = 0;
    }

    // This N will be chunk size now
    for (int i = 0; i < N; i++)
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
        sumx[min_index] += points[i].x;
        sumy[min_index] += points[i].y;
    }

    int anyChanged(Point * clusters, float *sumx, float *sumy, int *counts, int K)
    {
        int changed = 0;
        for (int i = 0; i < K; i++)
        {
            float x = sumx[i] / counts[i];
            float y = sumy[i] / counts[i];

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
        // if (argc < 3)
        //     return -1;

        // const int N = atoi(argv[1]);
        // const int K = atoi(argv[2]);
        const int N = 10000000;
        const int K = 4;

        int num_procs, rank;

        // Iniciating points and clusters
        Point *points = (Point *)malloc(N * sizeof(Point));      // points to be clustered
        int *global_count = (int *)malloc(K * sizeof(int));      // how many elements in clusters for rank 0
        float *global_sumx = (float *)malloc(K * sizeof(float)); // sum of distances x for rank 0
        float *global_sumy = (float *)malloc(K * sizeof(float)); // sum of distances y for rank 0
        int iterator = 0;                                        // How many times the algorithm has run
        Point *clusters = (Point *)malloc(K * sizeof(Point));    // centers of clusters
        int changing = 1;                                        // If the clusters are changing
        init(points, clusters, N, K);                            // Initializing points and clusters

        MPI_Init(&argc, &argv);
        MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);

        float *sumx = (float *)malloc(K * sizeof(float)); // sum of distances in x
        float *sumy = (float *)malloc(K * sizeof(float)); // sum of distances in y
        int *count = (int *)malloc(K * sizeof(Point));    // how many elements in clusters

        int chunk_size = N / num_procs; // Size of each chunk

        // Creating new type for sending points
        MPI_Datatype tmp_type, type_point;
        MPI_Aint lb, extent;

        int number = 1; // Says how many kinds of data your structure has

        // Says the type of every block
        MPI_Datatype array_of_types[] = {MPI_FLOAT};

        // Says how many elements for block
        int array_of_blocklengths[] = {2};

        /* Says where every block starts in memory, counting from the beginning of the struct. */
        // MPI_Aint array_of_displaysments[1] = {offsetof( Point, x ), offsetof( Point, y )};
        MPI_Aint address1, address2;
        Point _aux;
        MPI_Get_address(&_aux, &address1);
        MPI_Get_address(&_aux.x, &address2);
        MPI_Aint array_of_displaysments[] = {address2 - address1};

        MPI_Type_create_struct(
            number,
            array_of_blocklengths,
            array_of_displaysments,
            array_of_types,
            &tmp_type);
        MPI_Type_get_extent(tmp_type, &lb, &extent);
        MPI_Type_create_resized(tmp_type, lb, extent, &type_point);
        MPI_Type_commit(&type_point);

        // Scattering the points throughout the threads
        Point *local_points = (Point *)malloc(chunk_size * sizeof(Point)); // Chunk of points

        // Scatter the random numbers to all processes
        MPI_Scatter(
            points, chunk_size, type_point,
            local_points, chunk_size, type_point,
            0, MPI_COMM_WORLD);

        while (changing && iterator <= 40)
        {

            MPI_Bcast(clusters, K, type_point, 0, MPI_COMM_WORLD);

            kmeans(local_points, clusters, sumx, sumy, count, chunk_size, K);

            // If its not the main process, then it needs to end its results

            MPI_Reduce(&count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
            MPI_Reduce(&sumx, &global_sumx, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
            MPI_Reduce(&sumy, &global_sumy, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);

            // If it is, then we need to gather all the results (sums and counts)
            if (rank == 0)
            {
                // After gathering, we need to check for changes
                changing = anyChanged(clusters, sumx, sumy, count, K);
                iterator++;
            }
        }

        if (rank == 0) // If it is the main process, then we need to print the resultss
        {
            printf("N = %d, K = %d\n", N, K);
            for (int i = 0; i < K; i++)
            {
                printf("Center: (%.3f, %.3f) %d\n", clusters[i].x, clusters[i].y, count[i]);
            }
            printf("Iterations: %d times \n", iterator);
        }

        free(local_points);
        free(clusters);
        free(count);
        MPI_Type_free(&type_point);
        MPI_Finalize();
        free(points);

        return 0;
    }
