
#include <stdio.h>
#include <stdlib.h>

#define K 4
#define N 100000

typedef struct PointStruct {
	float x, y;
} *pPoint, Point;

typedef struct ClusterStruct {
	Point centroid;
	pPoint *samples;
	int size;
} *pCluster, Cluster;

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
	return (a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y);
}

void populateSamples(Point arr[])
{
	for(int i = 0; i < N; i++)
	{
		arr[i].x = ((float) rand() / (float) (RAND_MAX));
		arr[i].y = ((float) rand() / (float) (RAND_MAX));
	}
}

void initCluster(pCluster c, Point p)
{
	c->samples = malloc(N * sizeof(pPoint));
	c->centroid.x = p.x;
	c->centroid.y = p.y;
	c->size = 0;
}

void initClusters(Cluster clusters[], Point samples[])
{
	for(int i = 0; i < K; i++)
		initCluster(&clusters[i], samples[i]);
}

void appendPoint(pCluster c, pPoint p)
{
	c->samples[c->size] = p;
	c->size++;
}

void assignSamples(Cluster clusters[], Point samples[])
{
	for(int i = 0; i < K; i++)
		clusters[i].size = 0;

	for(int i = 0; i < N; i++)
	{
		int nearest_idx = 0;
		for(int j = 0; j < K; j++)
		{
			if(eucDist(samples[i],clusters[j].centroid) < eucDist(samples[i], clusters[nearest_idx].centroid))
				nearest_idx = j;
		}
		appendPoint(&clusters[nearest_idx], &samples[i]);
	}
}

int isIn(pPoint p, pPoint arr[], int size)
{
	for(int i = 0; i < size; i++)
		if(p == arr[i])
			return 1;
	return 0;
}

int reassignSamples(Cluster clusters[], Point samples[])
{
	Cluster clusters_tmp[K];
	for(int i = 0; i < K; i++)
	{
		clusters_tmp[i].centroid.x = clusters[i].centroid.x;
		clusters_tmp[i].centroid.y = clusters[i].centroid.y;
		clusters_tmp[i].size = 0;
		clusters_tmp[i].samples = malloc(N * sizeof(pPoint));
	}

	assignSamples(clusters_tmp, samples);

	int changed = 0;
	for(int i = 0; i < K; i++)
		for(int j = 0; j < clusters_tmp[i].size; j++)
			if(!isIn(clusters_tmp[i].samples[j], clusters[i].samples, clusters[i].size))
				changed++;

	for(int i = 0; i < K; i++)
		free(clusters_tmp[i].samples);

	if(changed>0)
		assignSamples(clusters, samples);

	return changed;
}

void updateCentroid(pCluster c)
{
	c->centroid.x = 0.f;
	c->centroid.y = 0.f;

	for(int i = 0; i < c->size; i++)
	{
		c->centroid.x += c->samples[i]->x;
		c->centroid.y += c->samples[i]->y;
	}

	c->centroid.x = c->centroid.x / (float) c->size;
	c->centroid.y = c->centroid.y / (float) c->size;
}

void updateCentroids(Cluster clusters[])
{
	for(int i = 0; i < K; i++)
		updateCentroid(&clusters[i]);
}

int main()
{
	//time_t t;
	//srand((unsigned) time(&t));
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
		changed = reassignSamples(clusters, samples);
		updateCentroids(clusters);

	} while(changed>0);

	printf("N = %d, K = %d\n", N, K);
	for(int i = 0; i < K; i++)
		printCluster(clusters[i]);
	printf("Iterations: %d\n", iterations);

	free(samples);
	return 0;
}
