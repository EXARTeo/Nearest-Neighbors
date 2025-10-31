#pragma once
#include <vector>
#include <cmath>

template <class T>
double compute_silhouette(
    const std::vector<std::vector<T>>& X,            //The point set
    const std::vector<int>& labels,                  //Cluster id for each Point
    const std::vector<std::vector<T>>& centroids     //The centroid set
);
