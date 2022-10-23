
#include <stdio.h>
#include <stdlib.h>

#define K 4
#define N 10000000

typedef struct PointStruct
{
	float x, y;
	int assigned_cluster;
} * pPoint, Point;

typedef struct ClusterStruct
{
	Point centroid;
	int size;
} * pCluster, Cluster;

void printPoint(Point p)
{
	printf("(%f, %f)", p.x, p.y);
}

void printCluster(Cluster c)
{
	printf("( ");
	printPoint(c.centroid);
	printf(", [ %d ] )\n", c.size);
}

float eucDist(Point a, Point b)
{
	return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
}

void populateSamples(Point arr[])
{
	for (int i = 0; i < N; i++)
	{
		arr[i].x = ((float)rand() / (float)(RAND_MAX));
		arr[i].y = ((float)rand() / (float)(RAND_MAX));
		arr[i].assigned_cluster = -1;
	}
}

void initCluster(pCluster c, Point p)
{
	c->centroid.x = p.x;
	c->centroid.y = p.y;
	c->size = 0;
}

void initClusters(Cluster clusters[], Point samples[])
{
	for (int i = 0; i < K; i++)
		initCluster(&clusters[i], samples[i]);
}

int assignSamples(Cluster clusters[], Point samples[])
{
	int changed = 0;
	for (int i = 0; i < N; i++)
	{
		int nearest_idx = 0;
		for (int j = 0; j < K; j++)
		{
			if (eucDist(samples[i], clusters[j].centroid) < eucDist(samples[i], clusters[nearest_idx].centroid))
				nearest_idx = j;
		}
		if (samples[i].assigned_cluster == nearest_idx)
			continue;
		else
		{
			clusters[samples[i].assigned_cluster].size--;
			samples[i].assigned_cluster = nearest_idx;
			clusters[nearest_idx].size++;
			changed++;
		}
	}
	return changed;
}

void updateCentroids(Cluster clusters[], Point samples[])
{
	Cluster new_clusters[K];
	for (int i = 0; i < K; i++)
	{
		new_clusters[i].centroid.x = 0.f;
		new_clusters[i].centroid.y = 0.f;
	}

	for (int i = 0; i < N; i++)
	{
		new_clusters[samples[i].assigned_cluster].centroid.x += samples[i].x;
		new_clusters[samples[i].assigned_cluster].centroid.y += samples[i].y;
	}

	for (int i = 0; i < K; i++)
	{
		clusters[i].centroid.x = new_clusters[i].centroid.x / (float)clusters[i].size;
		clusters[i].centroid.y = new_clusters[i].centroid.y / (float)clusters[i].size;
	}
}

int main()
{
	srand(10);

	Point *samples = malloc(N * sizeof(Point));
	Cluster clusters[K];

	populateSamples(samples);
	initClusters(clusters, samples);
	assignSamples(clusters, samples);

	int changed, iterations = 0;

	do
	{
		iterations++;
		updateCentroids(clusters, samples);
		changed = assignSamples(clusters, samples);

	} while (changed > 0);

	printf("N = %d, K = %d\n", N, K);
	for (int i = 0; i < K; i++)
		printCluster(clusters[i]);
	printf("Iterations: %d\n", iterations);

	free(samples);
	return 0;
}
