#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cuda_runtime.h>
#include <device_launch_parameters.h>

typedef struct Point
{
    float x, y;
} Point;

__global__ void compute_distances(Point *points, Point *clusters, int *count, float *dists, int *indices, const int N, const int K)
{
    int tid = blockIdx.x * blockDim.x + threadIdx.x;
    if (tid >= N)
        return;
    float min_value = 10000;
    int min_index = 0;

    for (int i = 0; i < K; i++)
    {
        float distx = points[tid].x - clusters[i].x;
        float disty = points[tid].y - clusters[i].y;
        float dist = distx * distx + disty * disty;
        if (dist < min_value)
        {
            min_value = dist;
            min_index = i;
        }
    }

    dists[tid] = min_value;
    indices[tid] = min_index;
}

__global__ void update_clusters(Point *clusters, int *count, float *sum_dist_x, float *sum_dist_y, const int K)
{
    int tid = blockIdx.x * blockDim.x + threadIdx.x;
    if (tid >= K)
        return;
    float x = sum_dist_x[tid] / count[tid];
    float y = sum_dist_y[tid] / count[tid];

    clusters[tid].x = x;
    clusters[tid].y = y;
}

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

int kmeans(Point *points, Point *clusters, int *count, const int N, const int K)
{
    int changed = 0;
    int *indices;
    float *dists;
    cudaMalloc((void **)&indices, N * sizeof(int));
    cudaMalloc((void **)&dists, N * sizeof(float));

    int block_size = 32;
    int num_blocks = (N + block_size - 1) / block_size;
    compute_distances<<<num_blocks, block_size>>>(points, clusters, count, dists, indices, N, K);

    cudaMemcpy(indices, indices, N * sizeof(int), cudaMemcpyDeviceToHost);
    cudaMemcpy(dists, dists, N * sizeof(float), cudaMemcpyDeviceToHost);

    float sum_dist_x[K];
    float sum_dist_y[K];
    for (int i = 0; i < K; i++)
    {
        count[i] = 0;
        sum_dist_x[i] = 0;
        sum_dist_y[i] = 0;
    }
    for (int i = 0; i < N; i++)
    {
        int min_index = indices[i];
        count[min_index]++;
        sum_dist_x[min_index] += points[i].x;
        sum_dist_y[min_index] += points[i].y;
    }

    cudaMemcpy(sum_dist_x, sum_dist_x, K * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(sum_dist_y, sum_dist_y, K * sizeof(float), cudaMemcpyHostToDevice);

    update_clusters<<<num_blocks, block_size>>>(clusters, count, sum_dist_x, sum_dist_y, K);

    cudaMemcpy(clusters, clusters, K * sizeof(Point), cudaMemcpyDeviceToHost);

    for (int i = 0; i < K; i++)
    {
        if (clusters[i].x != clusters[i].x || clusters[i].y != clusters[i].y)
        {
            changed = 1;
            break;
        }
    }

    cudaFree(indices);
    cudaFree(dists);

    return changed;
}

int main(int argc, char **argv)
{
    if (argc < 4)
        return -1;
    const int N = atoi(argv[1]);
    const int K = atoi(argv[2]);

    Point *points = malloc(N * sizeof(Point));
    Point *clusters = malloc(K * sizeof(Point));
    int *count = malloc(K * sizeof(Point));
    int iterator = 0;

    cudaError_t error = cudaSuccess;
    error = cudaSetDevice(0);
    if (error != cudaSuccess)
    {
        fprintf(stderr, "Failed to set device: %s\n", cudaGetErrorString(error));
        return -1;
    }

    init(points, clusters, N, K);

    do
    {
        iterator++;
    } while (kmeans(points, clusters, count, N, K) && iterator < 20);

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