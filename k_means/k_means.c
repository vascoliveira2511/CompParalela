#include <stdlib.h>

#define N 10000000
#define K 4

// Exemple of the matrix of points:
//  [x_0, y_0]
//  [x_1, y_1]
//  [..., ...]
//  [x_n, y_n]

void init(Point *points, Point *clusters)
{
    srand(10);
    for (int i = 0; i < (N * 2); i++)
    {
        points[i] = (float)rand() / RAND_MAX;
    }

    for (int i = 0; i < (K * 2); i++)
    {
        clusters[i] = points[i];
    }
}

int kmeans(float *points, float *clusters, int *count)
{
    int changed = 0;
    Point sum_of_distances[K];

    for (int i = 0; i < K; i++)
    {
        count[i] = 0;
        sum_of_distances[i] = 0;
        sum_of_distances[i + 1] = 0;
    }

    for (int i = 0; i < N; i += 2)
    {
        // de onde vem este min??
        float min = 1000000000;
        int min_index = 0;

        float x = points[i];
        float y = points[i + 1].y;

        // esta secção vai correr N*K vezes
        for (int j = 0; j < K; j += 2)
        {
            float dist_x = x - clusters[j].x;
            float dist_y = y - clusters[j + 1].y;

            float dist = dist_x * dist_x + dist_y * dist_y;

            if (dist < min)
            {
                min = dist;
                min_index = j;
            }
        }
        count[min_index]++;
        sum_of_distances[min_index].x += points[i].x;
        sum_of_distances[min_index].y += points[i + 1].y;
    }

    for (int i = 0; i < K; i++)
    {
        float x = sum_of_distances[i * 2 - 1].x / count[i];
        float y = sum_of_distances[i * 2].y / count[i];

        if (clusters[i * 2 - 1].x != x || clusters[i * 2].y != y)
        {
            clusters[i * 2 - 1] = x;
            clusters[i * 2] = y;
            changed = 1;
        }
    }

    return changed;
}

int main()
{
    float *points = malloc(N * 2 * sizeof(float));
    // float *py = (float *)malloc(N * sizeof(float));
    float clusters[K * 2];
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
        printf("Cluster %d: (%f, %f) \n", i, clusters[i * 2 - 1].x, clusters[i * 2].y);
        printf("Count: %d \n", count[i]);
    }

    free(points);

    return 0;
}