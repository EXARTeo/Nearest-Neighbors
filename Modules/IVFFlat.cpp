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

// -----------------------------
//  Helper Functions
// -----------------------------

//Returns point q's 'nprobe' closest centroids 
template <class T>
vector<int> nprobe_nearest_centroids(const vector<T>& q, const vector<vector<T>>& centroids, int nprobe){
    priority_queue<pair<double, int>> max_heap; //(dist, id)

    for (int i = 0 ; i < static_cast<int>(centroids.size()) ; i++){
        const vector<T>& centroid = centroids[i];
        double dist = lp_dist(q.begin(), q.end(), centroid.begin(), 2.0); //L2
        if ((int)max_heap.size() < nprobe){
            max_heap.emplace(dist, i);
        }
        else if (dist < max_heap.top().first) {
            max_heap.pop();
            max_heap.emplace(dist, i);
        }
    }

    vector<int> res;
    while (!max_heap.empty()) {
        res.emplace_back(max_heap.top().second);
        max_heap.pop();
    }
    reverse(res.begin(), res.end());

    return res;
}

//Returns a pair containing the distance and the index of the closest centroid to point q
template <class T>
pair<double, int> distance_to_nearest_centroid_2(const vector<T>& q, const vector<vector<T>>& centroids){
    double true_shortest_dist = -1.0;
    int cent_idx = 0;
    for (int i = 0 ; i < static_cast<int>(centroids.size()) ; i++){
        const vector<T>& centroid = centroids[i];
        double dist = lp_dist(q.begin(), q.end(), centroid.begin(), 2.0); //L2
        if (dist < true_shortest_dist || true_shortest_dist < 0.0){
            true_shortest_dist = dist;
            cent_idx = i;
        }
    }
    pair<double, int> return_val{true_shortest_dist, cent_idx};
    return return_val;
}

//Returns the distance to the closest centroid of point q
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

    if (X.size() == 0) throw runtime_error("The \"-d\" input dataset is empty");
    if (kclusters > static_cast<int>(X.size())) throw runtime_error("kclusters must be smaller than dataset size");

    vector<int> centroid_idxs;          //vector containing the indexes (aka object_id) of the centroid points
    centroid_idxs.reserve(kclusters);

    mt19937_64 rng(static_cast<uint64_t>(seed));
    uniform_int_distribution<int> centroid_rand(0, static_cast<int>(X.size()) - 1);
    centroid_idxs.push_back(centroid_rand(rng));

    //i = 1 at start because we already got 1 centroid
    for(int i = 1 ; i < kclusters ; i++){

        double total = 0.0;                         //sum of all D(i)^2 
        vector<double> all_squared_dists;           //squared distance of each point to closest centroid
        all_squared_dists.resize(X.size(), 0.0);    //initialize all distances at zero

        for (int j = 0 ; j < static_cast<int>(X.size()) ; j++){
            if (find(centroid_idxs.begin(), centroid_idxs.end(), j) != centroid_idxs.end()){
                continue;                           //if X[j] is a centroid, skip it
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

        //Due to floating point error, cumulative may never exceed r
        if (static_cast<int>(centroid_idxs.size()) < i + 1)
            centroid_idxs.push_back(X.size() - 1);          //Fallback

    }
    return centroid_idxs;
}

template <class T>
vector<vector<T>> lloyds_alg(const vector<vector<T>>& X, vector<int>& centroid_idxs, int kclusters, uint32_t seed){
    //convert initial centroid indices into actual centroid vectors
    vector<vector<T>> centroids;
    centroids.reserve(kclusters);
    for (int idx : centroid_idxs)
        centroids.push_back(X[idx]);

    const int max_iters = 100;              //max iterations before stopping
    const double tol = 1e-4;                //convergence tolerance
    int dim = X[0].size();
    vector<int> assignments(X.size(), -1);  //contains the index for the closest centroid of each point

    for (int iter = 0 ; iter < max_iters ; iter++){
        //assign each point to nearest centroid
        for (int i = 0 ; i < static_cast<int>(X.size()) ; i++){
            const vector<T>& temp = X[i];
            pair<double, int> dist_and_cent = distance_to_nearest_centroid_2(temp, centroids);
            assignments[i] = dist_and_cent.second;
        }

        //recompute centroids
        vector<vector<T>> new_centroids(kclusters, vector<T>(dim, 0));
        vector<int> counts(kclusters, 0);

        
        for (int i = 0 ; i < static_cast<int>(X.size()) ; i++){
            int cluster = assignments[i];
            counts[cluster]++;
            for (int d = 0; d < dim; ++d)
                new_centroids[cluster][d] += X[i][d];
        }
        
        for (int cluster = 0 ; cluster < static_cast<int>(new_centroids.size()) ; cluster++){
            if (counts[cluster] > 0){
                for (int d = 0; d < dim; ++d)
                    new_centroids[cluster][d] =  new_centroids[cluster][d] / static_cast<double>(counts[cluster]);
            }
            else {
                mt19937_64 rng(static_cast<uint64_t>(seed));
                uniform_int_distribution<int> subset_rand(0, static_cast<int>(X.size()) - 1);
                int idx = subset_rand(rng);
                new_centroids[cluster] = X[idx];   //TODO Yolo move goes brrrr
            }
        }

        //check convergence
        double max_shift = 0.0;
        for (int c = 0; c < kclusters; ++c) {
            double shift = lp_dist(centroids[c].begin(), centroids[c].end(), new_centroids[c].begin(), 2.0);
            max_shift = max(max_shift, shift);
        }

        centroids.swap(new_centroids);

        if (max_shift < tol) {
            break;
        }
    }

    return centroids;
}

// -----------------------------
//  IVFFlat Implementation
// -----------------------------

template <class T>
IVFFlat<T>::IVFFlat(int kclusters, int nprobe, uint32_t seed)
    : seed(seed), kclusters(kclusters), nprobe(nprobe){    
    if (kclusters <= 0) throw invalid_argument("kclusters must be positive");
    if (nprobe <= 0)    throw invalid_argument("nprobe must be positive");

    table.buckets.resize(kclusters);
}

template <class T>
void IVFFlat<T>::insert_object(uint32_t obj_id, const vector<T>& x, const vector<vector<T>>& centroids) {
    pair<double, int> dist_and_cent = distance_to_nearest_centroid_2(x, centroids);
    int closest_centroid = dist_and_cent.second;

    Entry<T> new_entry{obj_id, &x, static_cast<uint32_t>(closest_centroid)};
    table.buckets[closest_centroid].push_back(new_entry);
}

template <class T>
void IVFFlat<T>::build(const vector<vector<T>>& X) {
    if (kclusters <= 0) throw runtime_error("kclusters must be positive integer");
    if (kclusters > static_cast<int>(X.size()))
        kclusters = static_cast<int>(X.size());

    //Fisrt, create a subset of X, X'
    int sqrt_n = static_cast<int>(floor(sqrt(static_cast<double>(X.size()))));  //get floor of square root of X.size()
    if (sqrt_n < kclusters)
        sqrt_n = kclusters;

    vector<vector<T>> subset;                                                   //stores a random subset X' of X with sqrt(X.size()) elements
    subset.reserve(sqrt_n);

    unordered_set<int> chosen_indices;                                          //store the indices of the already chosen elements for the subset

    mt19937_64 rng(static_cast<uint64_t>(seed));
    uniform_int_distribution<int> subset_rand(0, static_cast<int>(X.size()) - 1);

    while (static_cast<int>(subset.size()) < sqrt_n) {
        int idx = subset_rand(rng);
        if (chosen_indices.insert(idx).second) {                                //add element to subset only if it is new
            subset.push_back(X[idx]);
        }
    }

    //compute kmeans++ init to get first approximation of centroids first
    vector<int> centroid_idxs = kmeans_init(subset, kclusters, static_cast<int>(seed));

    //now have to do Lloyd's algorithm to get the final centroids
    final_centroids = lloyds_alg(subset, centroid_idxs, kclusters, seed);

    //Lastly insert all objects
    for (uint32_t id = 0; id < X.size(); ++id)
        insert_object(id, X[id], final_centroids);
}

// -----------------------------
//  k-NN Query
// -----------------------------

template <class T>
vector<pair<uint32_t, double>> IVFFlat<T>::query_knn(const vector<T>& q, int N) const {

    vector<int> cent_idxs = nprobe_nearest_centroids(q, final_centroids, nprobe);   //return vector with idxs of nprobe nearest clusters (aka bucket idxs)
    priority_queue<pair<double, uint32_t>> max_heap;                                //(dist, id)

    for(int i = 0 ; i < static_cast<int>(cent_idxs.size()) ; i++){
        const auto& bucket = table.buckets[cent_idxs[i]];
        for (const auto& e : bucket) {
            double dist = lp_dist(e.x->begin(), e.x->end(), q.begin(), 2.0); //L2
            if ((int)max_heap.size() < N)
                max_heap.emplace(dist, e.obj_id);
            else if (dist < max_heap.top().first) {
                max_heap.pop();
                max_heap.emplace(dist, e.obj_id);
            }
        }
    }

    vector<pair<uint32_t, double>> res;
    while (!max_heap.empty()) {
        res.emplace_back(max_heap.top().second, max_heap.top().first);
        max_heap.pop();
    }
    reverse(res.begin(), res.end());

    return res;
}

// -----------------------------
//  Range Query
// -----------------------------

template <class T>
vector<uint32_t> IVFFlat<T>::query_range(const vector<T>& q, double R, size_t max_checked) const {

    vector<int> cent_idxs = nprobe_nearest_centroids(q, final_centroids, nprobe);   //return vector with idxs of nprobe nearest clusters (aka bucket idxs)
    vector<uint32_t> res;
    size_t max_check = 0;

    for(int i = 0 ; i < static_cast<int>(cent_idxs.size()) ; i++){
        const auto& bucket = table.buckets[cent_idxs[i]];
        for (const auto& e : bucket) {
            double dist = lp_dist(e.x->begin(), e.x->end(), q.begin(), 2.0); //L2
            if (dist <= R){
                res.push_back(e.obj_id);
            }
            max_check++;
            if (max_checked && max_check >= max_checked)
                break;
        }
        if (max_checked &&max_check >= max_checked)
            break;
    }

    return res;
}