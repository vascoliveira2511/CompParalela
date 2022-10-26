#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 10000000
#define K 4

void init(float *px, float *py, float *cx, float *cy)
{
    srand(10);
    for (int i = 0; i < N; i++)
    {
        px[i] = (float)rand() / RAND_MAX;
        py[i] = (float)rand() / RAND_MAX;
    }

    for (int i = 0; i < K; i++)
    {
        cx[i] = px[i];
        cy[i] = py[i];
    }
}

int kmeans(float *px, float *py, float *cx, float *cy, int *count)
{
    int changed = 0;
    float sum_x[K];
    float sum_y[K];

    for (int i = 0; i < K; i++)
    {
        count[i] = 0;
        sum_x[i] = 0;
        sum_y[i] = 0;
    }

    for (int i = 0; i < N; i++)
    {
        // de onde vem este min??
        float min = 10000;
        int min_index = 0;

        float aux_x = px[i];
        float aux_y = py[i];

        // esta secção vai correr N*K vezes
        for (int j = 0; j < K; j++)
        {
            aux_x -= cx[j];
            aux_y -= cy[j];

            float dist = aux_x * aux_x + aux_y * aux_y;

            min_index = dist < min ? j : min_index;
            min = dist < min ? dist : min;
        }
        count[min_index]++;
        sum_x[min_index] += px[i];
        sum_y[min_index] += py[i];
    }

    for (int i = 0; i < K; i++)
    {
        float x = sum_x[i] / count[i];
        float y = sum_y[i] / count[i];

        if (cx[i] != x || cy[i] != y)
        {
            cx[i] = x;
            cy[i] = y;
            changed = 1;
        }
    }

    return changed;
}

int main()
{
    float *px = (float *)malloc(N * sizeof(float));
    float *py = (float *)malloc(N * sizeof(float));
    float cx[K];
    float cy[K];

    int iterator = 0;
    int count[K];

    init(px, py, cx, cy);

    do
    {
        iterator++;
    } while (kmeans(px, py, cx, cy, count));

    printf("Iterations: %d times \n ", iterator);
    for (int i = 0; i < K; i++)
    {
        printf("Cluster %d: (%f, %f) \n", i, cx[i], cy[i]);
        printf("Count: %d \n", count[i]);
    }

    free(px);
    free(py);

    return 0;
}