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
    float sumx[K];
    float sumy[K];

    for (int i = 0; i < K; i++)
    {
        count[i] = 0;
        sumx[i] = 0;
        sumy[i] = 0;
    }

    for (int i = 0; i < N; i++)
    {
        float min = 1000000;
        int min_index = 0;

        // esta secção vai correr N*K vezes
        for (int j = 0; j < K; j++)
        {
            float distx = px[i] - cx[i];
            float disty = py[i] - cy[j];

            float dist = distx * distx + disty * disty;

            min = dist < min ? dist : min;          // min = min(dist, min)
            min_index = dist < min ? j : min_index; // min_index = min(dist, min_index)
        }
        count[min_index]++;
        sumx[min_index] += px[i];
        sumy[min_index] += py[i];
    }

    for (int i = 0; i < K; i++)
    {
        float x = sumx[i] / count[i];
        float y = sumy[i] / count[i];

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