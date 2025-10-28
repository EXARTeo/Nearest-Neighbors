#include <unordered_set>
#include <queue>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <functional>

#include "../Includes/IVFFlat.hpp"
#include "../Common/Vector_distance.hpp"

using namespace std;

template <class T>
double distance_to_nearest_centroid(const vector<vector<T>>& X, const vector<T>& q, const vector<int>& centroid_idxs){
    double true_shortest_dist = -1.0;
    for (int i = 0 ; i < static_cast<int>(centroid_idxs.size()) ; i++){
        const vector<T>& centroid = X[centroid_idxs[i]];
        double dist = lp_dist(q.begin(), q.end(), centroid.begin(), 2.0); //L2
        if (dist < true_shortest_dist || true_shortest_dist < 0.0){
            true_shortest_dist = dist;
        }
    }
    return true_shortest_dist;
}

template <class T>
vector<int> kmeans_init(const vector<vector<T>>& X, int kclusters, int seed){
    // //possible improvement: check if X.size() == 0 or kclusters == 0 or kclusters > X.size() and throw error
    // if (X.size() == 0) return;
    // if (kclusters == 0) return;   ////////////// IMPORTANT: handle these cases correctly
    // if (kclusters > X.size()) return;

    vector<int> centroid_idxs; //vector containing the indexes (aka object_id) of the centroid points
    centroid_idxs.reserve(kclusters);

    mt19937_64 rng(static_cast<uint64_t>(seed));
    uniform_int_distribution<int> centroid_rand(0, static_cast<int>(X.size()) - 1);
    centroid_idxs.push_back(centroid_rand(rng));

    //i = 1 at start because we already got 1 centroid
    for(int i = 1 ; i < kclusters ; i++){

        double total = 0.0; //sum of all D(i)^2 
        vector<double> all_squared_dists; //squared distance of each point to closest centroid
        all_squared_dists.resize(X.size(), 0.0); //initialize all distances at zero

        for (int j = 0 ; j < static_cast<int>(X.size()) ; j++){
            if (find(centroid_idxs.begin(), centroid_idxs.end(), j) != centroid_idxs.end()){
                continue; //if X[j] is a centroid, skip it
            }
            double shortest_dist = distance_to_nearest_centroid(X, X[j], centroid_idxs);

            all_squared_dists[j] = shortest_dist * shortest_dist;
            total += all_squared_dists[j];
        }

        //get random number from 0 to total
        uniform_real_distribution<double> dist(0.0, total);
        double r = dist(rng);

        //find the next centroid with probability proportional to D(i)^2
        double cumulative = 0.0;
        for (int j = 0 ; j < static_cast<int>(X.size()) ; j++){
            cumulative += all_squared_dists[j];
            if(cumulative >= r){
                centroid_idxs.push_back(j);
                break;
            }
        }
        // //due to floating point error, cumulative may never exceed r
        // if (centroid_idxs.size() < i + 1)
        //     centroid_idxs.push_back(X.size() - 1); //fallback

    }
    return centroid_idxs;
}

template <class T>
IVFFlat<T>::IVFFlat(int kclusters, int nprobe, uint32_t seed)
    : kclusters(kclusters), nprobe(nprobe), seed(seed){

    table.buckets.resize(kclusters);
}

template <class T>
void IVFFlat<T>::insert_object(uint32_t obj_id, const std::vector<T>& x, const vector<int>& centroid_idxs) {
    
}

template <class T>
void IVFFlat<T>::build(const vector<vector<T>>& X) {
    //compute kmeans++ init to get first approximation of centroids first
    vector<int> centroid_idxs = kmeans_init(X, kclusters, static_cast<int>(seed));

    //now have to do Lloyd's algorithm to get the final centroids
    //TODO

    //Lastly all insert objects (TODO)
    for (uint32_t id = 0; id < X.size(); ++id)
        insert_object(id, X[id], centroid_idxs);
}