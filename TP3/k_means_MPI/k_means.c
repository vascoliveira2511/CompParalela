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

int kmeans(Point *points, Point *clusters, Point *sums, int *count, const int N, const int K)
{
    int changed = 0;

    for (int i = 0; i < K; i++)
    {
        count[i] = 0;
        sums[i].x = 0;
        sums[i].y = 0;
    }

    //This N will be chunk size now
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
        sums[min_index].x += points[i].x;
        sums[min_index].y += points[i].y;
    }

}


int anyChanged(Point *clusters, Point *sums, int *counts, int K){
    int changed = 0;
    for (int i = 0; i < K; i++){
        float x = sums[i].x / counts[i];
        float y = sums[i].y / counts[i];

        if(clusters[i].x != x || clusters[i].y != y){
            clusters[i].x = x;
            clusters[i].y = y;
            changed = 1;
        }
    }
    return changed;
}

void addSum(Point *sumsOriginal, Point *sumsDollarStore, int K){
    for (int i = 0; i < K; i++){
        sumsOriginal[i].x += sumsDollarStore[i].x;
        sumsOriginal[i].y += sumsDollarStore[i].y;
    }
}

void addCount(int *countOriginal, int *countDollarStore, int K){
    for (int i = 0; i < K; i++){
        countOriginal[i] += countDollarStore[i];
    }
}



int main(int argc, char **argv)
{
    //if (argc < 3)
    //    return -1;

    //const int N = atoi(argv[1]);
    //const int K = atoi(argv[2]);
    const int N = 10000000;
    const int K = 4;

    int num_procs, rank;
    

    //Iniciating points and clusters
    Point *points = (Point*)malloc(N * sizeof(Point));

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    
    int changing = 1; //Only the thread 0 will have power over this
    Point *clusters = (Point*)malloc(K * sizeof(Point)); //centers of clusters
    Point *sums     = (Point*)malloc(K * sizeof(Point)); //sum of distances
    int   *count    = (int*)malloc(K * sizeof(Point)); //how many elements in clusters
    


    int chunk_size = N / num_procs;
    int start = rank * chunk_size; //start of current chunk
    int end = start + chunk_size;  //end of current chunk
   

    //Creating new type for sending points
    MPI_Datatype tmp_type, type_point;
    MPI_Aint lb, extent;

    int number = 1; //Says how many kinds of data your structure has

    // Says the type of every block
    MPI_Datatype array_of_types[] =  {MPI_FLOAT};

    // Says how many elements for block
    int array_of_blocklengths[] = {2};

    /* Says where every block starts in memory, counting from the beginning of the struct. */
    //MPI_Aint array_of_displaysments[1] = {offsetof( Point, x ), offsetof( Point, y )};
    MPI_Aint address1, address2;
    Point _aux;
    MPI_Get_address(&_aux,&address1);
    MPI_Get_address(&_aux.x,&address2);
    MPI_Aint array_of_displaysments[] = {address2 - address1};

    MPI_Type_create_struct( 
        number, 
        array_of_blocklengths, 
        array_of_displaysments,
        array_of_types,
        &tmp_type 
    );
    MPI_Type_get_extent( tmp_type, &lb, &extent );
    MPI_Type_create_resized( tmp_type, lb, extent, &type_point );
    MPI_Type_commit( &type_point );

    //Initiating points of the whole set
    if (rank == 0) init(points, clusters, N, K);

    //Scattering the points throughout the threads
    Point *local_points = (Point*)malloc(chunk_size * sizeof(Point)); //Chunk of points
    // Scatter the random numbers to all processes
    MPI_Scatter(
        points, chunk_size, type_point, 
        local_points, chunk_size, type_point, 
        0, MPI_COMM_WORLD
    );

    int iterator = 0;


    while(changing || iterator <= 40){

        kmeans(local_points, clusters, sums, count, chunk_size, K);

        //If its not the main process, then it needs to send its results
        if (rank != 0){
            
            MPI_Send(sums, K , type_point, 0, MPI_ANY_TAG, MPI_COMM_WORLD);
            
            MPI_Send(count, K , MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD);

        } 

        //If it is, then we need to gather all the results (sums and counts)
        if(rank == 0){
            MPI_Status status;
            Point sumAux[K];
            int countAux[K];

            for (int i = 1; i < num_procs; i++){
                MPI_Recv(sumAux, K, type_point, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                addSum(sums, sumAux, K);
                MPI_Recv(countAux, K, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                addCount(count, countAux, K);
                for (int i = 0; i < K; i++)
                {
                    printf("count: %d\n", count[i]);
                }
            }


            // After gathering, we need to check for changes
            changing = anyChanged(clusters, sums, count, K);

            iterator++;
        }

        // Nobody moves on untill thread 0 checks if anything changed
        MPI_Barrier(MPI_COMM_WORLD);
    }   

    if(rank == 0)
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
