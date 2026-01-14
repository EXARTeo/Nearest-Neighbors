#include <unordered_set>
#include <queue>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <functional>

#include "../Includes/IVFPQ.hpp"
#include "../Common/Vector_distance.hpp"

//Uncomment for testing silhouette (0)
// #include "../tests/Silhouette/Silhouette.hpp"

using namespace std;


// -----------------------------
//  Helper Functions
// -----------------------------

template <class ItX, class ItY>
double fast_dist(ItX x_start, ItX x_end, ItY y_start, double k){

    long double dist = 0.0L;

    for (; x_start != x_end; ++x_start, ++y_start) {
        long double my_pow = (long double)(*x_start) - (long double)(*y_start);
        dist += my_pow * my_pow;
    }
    return (double)dist;
}

template <class T>
vector<vector<T>> get_random_subset(const vector<vector<T>>& X, int cluster_num, int seed){
    if (cluster_num <= 0) throw runtime_error("cluster_num must be positive integer");
    if (cluster_num > static_cast<int>(X.size()))
        cluster_num = static_cast<int>(X.size());

    //Fisrt, create a subset of X, X'
    int sqrt_n = static_cast<int>(floor(sqrt(static_cast<double>(X.size()))));  //Get floor of square root of X.size()
    if (sqrt_n < cluster_num)
        sqrt_n = cluster_num;

    vector<vector<T>> subset;                       //Stores a random subset X' of X with sqrt(X.size()) elements
    subset.reserve(sqrt_n);

    unordered_set<int> chosen_indices;              //Store the indices of the already chosen elements for the subset
    mt19937_64 rng(static_cast<uint64_t>(seed));
    uniform_int_distribution<int> subset_rand(0, static_cast<int>(X.size()) - 1);

    while (static_cast<int>(subset.size()) < sqrt_n) {
        int idx = subset_rand(rng);
        if (chosen_indices.insert(idx).second) {    //Add element to subset only if it is new
            subset.push_back(X[idx]);
        }
    }
    return subset;
}

//Returns a pair containing the distance and the index of the closest centroid to point q
template <class T>
pair<double, int> nearest_centroid(const vector<T>& q, const vector<vector<float>>& centroids){
    double true_shortest_dist = -1.0;
    int cent_idx = 0;
    for (int i = 0 ; i < static_cast<int>(centroids.size()) ; i++){
        double dist = fast_dist(q.begin(), q.end(), centroids[i].begin(), 2.0); //L2
        if (dist < true_shortest_dist || true_shortest_dist < 0.0){
            true_shortest_dist = dist;
            cent_idx = i;
        }
    }
    pair<double, int> return_val{true_shortest_dist, cent_idx};
    return return_val;
}


//Returns point q's 'nprobe' closest centroids 
template <class T>
vector<int> nprobe_nearest_centroids(const vector<T>& q, const vector<vector<float>>& centroids, int nprobe){
    priority_queue<pair<double, int>> max_heap; //(dist, id)

    for (int i = 0 ; i < static_cast<int>(centroids.size()) ; i++){
        const vector<float>& centroid = centroids[i];
        double dist = fast_dist(q.begin(), q.end(), centroid.begin(), 2.0); //L2
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


//Returns the distance to the closest centroid of point q
template <class T>
double distance_to_nearest_centroid(const vector<vector<T>>& X, const vector<T>& q, const vector<int>& centroid_idxs){
    double true_shortest_dist = -1.0;
    for (int i = 0 ; i < static_cast<int>(centroid_idxs.size()) ; i++){
        double dist = fast_dist(q.begin(), q.end(), X[centroid_idxs[i]].begin(), 2.0); //L2
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

    vector<int> centroid_idxs;          //Vector containing the indexes (aka object_id) of the centroid points
    centroid_idxs.reserve(kclusters);

    mt19937_64 rng(static_cast<uint64_t>(seed));
    uniform_int_distribution<int> centroid_rand(0, static_cast<int>(X.size()) - 1);
    centroid_idxs.push_back(centroid_rand(rng));

    //i = 1 at start because we already got 1 centroid
    for(int i = 1 ; i < kclusters ; i++){

        double total = 0.0;                         //Sum of all D(i)^2 
        vector<double> all_squared_dists;           //Squared distance of each point to closest centroid
        all_squared_dists.resize(X.size(), 0.0);    //Initialize all distances at zero

        for (int j = 0 ; j < static_cast<int>(X.size()) ; j++){
            if (find(centroid_idxs.begin(), centroid_idxs.end(), j) != centroid_idxs.end()){
                continue;                           //if X[j] is a centroid, skip it
            }
            double shortest_dist = distance_to_nearest_centroid(X, X[j], centroid_idxs);

            all_squared_dists[j] = shortest_dist * shortest_dist;
            total += all_squared_dists[j];
        }

        //Get random number from 0 to total
        uniform_real_distribution<double> dist(0.0, total);
        double r = dist(rng);

        //Find the next centroid with probability proportional to D(i)^2
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
vector<vector<float>> lloyds_alg(const vector<vector<T>>& X, vector<int>& centroid_idxs, int kclusters, uint32_t seed){
    vector<vector<float>> centroids;
    centroids.reserve(centroid_idxs.size());
    for (const auto& idx : centroid_idxs)
        centroids.emplace_back(X[idx].begin(), X[idx].end());

    const int max_iters = 15;               //max iterations before stopping
    const double tol = 1e-4;                //convergence tolerance
    int dim = X[0].size();
    vector<int> assignments(X.size(), -1);  //contains the index for the closest centroid of each point

    for (int iter = 0 ; iter < max_iters ; iter++){
        //assign each point to nearest centroid
        for (int i = 0 ; i < static_cast<int>(X.size()) ; i++){
            pair<double, int> dist_and_cent = nearest_centroid(X[i], centroids);
            assignments[i] = dist_and_cent.second;
        }

        //recompute centroids
        vector<vector<float>> new_centroids(kclusters, vector<float>(dim, 0));
        vector<int> counts(kclusters, 0);

        for (int i = 0 ; i < static_cast<int>(X.size()) ; i++){
            int cluster = assignments[i];
            counts[cluster]++;
            for (int di = 0; di < dim; ++di)
                new_centroids[cluster][di] += static_cast<float>(X[i][di]);
        }

        for (int cluster = 0 ; cluster < static_cast<int>(new_centroids.size()) ; cluster++){
            if (counts[cluster] > 0){
                for (int di = 0; di < dim; ++di)
                    new_centroids[cluster][di] = new_centroids[cluster][di] / static_cast<float>(counts[cluster]);
            }
            else {
                mt19937_64 rng(static_cast<uint64_t>(seed));
                uniform_int_distribution<int> subset_rand(0, static_cast<int>(X.size()) - 1);
                int idx = subset_rand(rng);
                for (int i = 0 ; i < dim ; i++){
                    new_centroids[cluster][i] = static_cast<float>(X[idx][i]);
                }
            }
        }

        //check convergence
        double max_shift = 0.0;
        for (int c = 0; c < kclusters; ++c) {
            double shift = fast_dist(centroids[c].begin(), centroids[c].end(), new_centroids[c].begin(), 2.0);
            max_shift = max(max_shift, shift);
        }

        centroids.swap(new_centroids);
        if (max_shift == 0 || max_shift < tol) {
            break;
        }
    }

    return centroids;
}


// -----------------------------
//  IVFPQ Implementation
// -----------------------------

template <class T>
IVFPQ<T>::IVFPQ(size_t d, int kclusters, int nprobe, int M, int nbits, uint32_t seed)
: seed(seed), kclusters(kclusters), nprobe(nprobe), M(M), nbits(nbits), s(1 << nbits), d(d){

    if (kclusters <= 0)            throw invalid_argument("kclusters must be positive");
    if (nprobe <= 0)               throw invalid_argument("nprobe must be positive");
    if (nbits <= 0 || nbits >= 32) throw invalid_argument("nbits must be positive and nbits < 32");
    if (M <= 0 || (d%M != 0))      throw invalid_argument("M must be positive and d%M == 0");
    
    subdim = d/M;

    lists.resize(kclusters);
    residuals.resize(M);        //Residuals[m] = list of vectors (AKA r(x) = x - c(x))
}

//Uncomment for testing silhouette (1)
// vector<int> sil_testpq;

template <class T>
void IVFPQ<T>::insert_object(uint32_t obj_id, const vector<T>& x) {
    if (final_centroids.empty() || subspace_centroids.empty()) throw logic_error("IVFPQ: train before insert");
    if (x.size() != d) throw invalid_argument("IVFPQ: x dim mismatch");

    vector<float> xf(x.begin(), x.end());
    pair<double, int>to_second = nearest_centroid(xf, final_centroids);

    int j = to_second.second;

    //Uncomment for testing silhouette (2)
    // sil_testpq[obj_id] = j;

    vector<unsigned int> codes;
    codes.resize(M);

    vector<vector<float>> r;
    r.reserve(M);
    for (int i = 0; i < M; ++i)
        r.push_back(residuals[i][obj_id]);   //r_i(x_id)

    encode_residual(r, codes);

    lists[j].push_back(PQEntry{obj_id, std::move(codes), static_cast<uint32_t>(j)});
}

template <class T>
void IVFPQ<T>::build(const vector<vector<T>>& X) {

    vector<vector<T>> subset = get_random_subset(X, kclusters, seed);

    //Get first approximate centroids
    vector<int> centroid_idxs = kmeans_init(subset, kclusters, static_cast<int>(seed));

    vector<vector<float>> subset_f;
    subset_f.reserve(subset.size());
    for (const auto& v : subset)
        subset_f.emplace_back(v.begin(), v.end());

    //Lloyd's algorithm to get the final centroids
    final_centroids = lloyds_alg(subset_f, centroid_idxs, kclusters, seed);        
    //Build the subspace_centroids
    build_subcentroids(X);

    //Uncomment for testing silhouette (3)
    // sil_testpq.resize(X.size());

    //Lastly insert all objects
    for (uint32_t id = 0; id < X.size(); ++id)
        insert_object(id, X[id]);

    //Uncomment for testing silhouette (4)
    // vector<vector<float>> X_f;
    // X_f.reserve(X.size());
    // for (const auto& v : X)
    //     X_f.emplace_back(v.begin(), v.end());

    // cout <<"Silhouette: "<<compute_silhouette_parallel<float>(X_f, sil_testpq, final_centroids)<<'\n';
}

//Calculate the PQ(x) = [code_1(x), ..., code_M(x)] for x with residual r, and store it in codes
template <class T>
void IVFPQ<T>::encode_residual(const vector<vector<float>>& r, vector<unsigned int>& codes) const {
    for (int m = 0; m < M; ++m) {
        int best_h = 0;
        double best = fast_dist(r[m].begin(), r[m].end(), subspace_centroids[m][0].begin(), 2.0);       //L2
        for (int h = 1; h < s; ++h) {
            double dist = fast_dist(r[m].begin(), r[m].end(), subspace_centroids[m][h].begin(), 2.0);   //Dist between r_i and c_i,h
            if (dist < best) {
                best = dist;
                best_h = h;
            }
        }
        codes[m] = static_cast<unsigned int>(best_h);
    }
}

//calculate the residual of point x with centroid final_centroids[j] and store directly in r
template <class T>
void IVFPQ<T>::compute_residual(const vector<T>& x, int j, vector<float>& r) const {
    const auto& cj = final_centroids[j];
    for (size_t i = 0; i < d; ++i)
        r[i] = static_cast<float>(x[i]) - static_cast<float>(cj[i]);
}

template <class T>
void IVFPQ<T>::build_subcentroids(const vector<vector<T>>& X) {
    if (final_centroids.empty()) throw logic_error("IVFPQ: coarse centroids not set");
    if (d <= 0 || subdim <= 0) throw logic_error("IVFPQ: invalid dims");

    for (int m = 0; m < M; ++m)
        residuals[m].reserve(X.size());

    vector<float> r;
    r.resize(d);                                                //One point's full residual
    for (const auto& x : X) {
        vector<float> xf(x.begin(), x.end());
        pair<double, int> to_second = nearest_centroid(xf, final_centroids);
        int j = to_second.second;
        compute_residual(x, j, r);                              //Calculate point x's residual with centroid j and store in r
        for (int m = 0; m < M; ++m) {
            vector<float> part(subdim);                         //Store each r_i
            int base = m * subdim;                              //Which index of the points coordinates we start at for r_i
            for (int t = 0; t < subdim; ++t)
                part[t] = r[base + t];
            residuals[m].push_back(move(part));
        }
    }

    subspace_centroids.assign(M, {});
    //For each subspace..
    for (int m = 0; m < M; ++m) {
        //..initialize for each c_m all the h
        subspace_centroids[m].assign(s, vector<float>(subdim, 0));

        vector<vector<float>> centers;
        vector<vector<float>> rand_set = get_random_subset(residuals[m], s, seed);

        //Call lloyd for each subspace..
        vector<int> idx = kmeans_init(rand_set, s, seed);
        centers = lloyds_alg(rand_set, idx, s, seed);
        //..and save the sub centroids
        for (int h = 0; h < s; ++h)
            subspace_centroids[m][h] = move(centers[h]);
    }
}


// -----------------------------
//  k-NN Query
// -----------------------------

template <class T>
vector<pair<uint32_t, double>> IVFPQ<T>::query_knn(const vector<T>& q, int N) const {

    vector<int> cent_idxs = nprobe_nearest_centroids(q, final_centroids, nprobe);   //Return vector with idxs of nprobe nearest clusters (aka bucket idxs)
    priority_queue<pair<double, uint32_t>> max_heap;    //(dist, id)

    for(int i = 0 ; i < static_cast<int>(cent_idxs.size()) ; i++){
        vector<vector<double>> LUT;
        LUT.resize(M, vector<double>(s, -1.0));

        vector<float> r;
        r.resize(d);
        compute_residual(q, cent_idxs[i], r);

        vector<vector<float>> all_res_parts;    //r(q) = [r_1(q), ... , r_M(q)]
        all_res_parts.reserve(M);

        for (int m = 0; m < M; ++m) {
            vector<float> part;                 //Store each r_i
            part.resize(subdim);
            int base = m * subdim;              //Which index of the points coordinates we start at for r_i
            for (int t = 0; t < subdim; ++t)
                part[t] = r[base + t];
            all_res_parts.push_back(move(part));
        }

        const auto& bucket = lists[cent_idxs[i]];
        for (const auto& e : bucket) {
            double dist = 0.0;
            for (int m = 0; m < M; ++m) {
                unsigned int code = e.codes[m];
                if(LUT[m][code] < 0.0){
                    LUT[m][code] = fast_dist(all_res_parts[m].begin(), all_res_parts[m].end(), subspace_centroids[m][code].begin(), 2.0);
                }
                dist += LUT[m][code];
            }

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
    for (auto &p : res)
        p.second = sqrt(p.second);

    return res;
}

// // -----------------------------
// //  Range Query
// // -----------------------------

template <class T>
vector<uint32_t> IVFPQ<T>::query_range(const vector<T>& q, double R, size_t max_checked) const {

    vector<int> cent_idxs = nprobe_nearest_centroids(q, final_centroids, nprobe);   //return vector with idxs of nprobe nearest clusters (aka bucket idxs)
    vector<uint32_t> res;
    size_t max_check = 0;

    for(int i = 0 ; i < static_cast<int>(cent_idxs.size()) ; i++){

        vector<vector<double>> LUT;
        LUT.resize(M, vector<double>(s, -1.0));

        vector<float> r;
        r.resize(d);
        compute_residual(q, cent_idxs[i], r);

        vector<vector<float>> all_res_parts;
        all_res_parts.reserve(M);

        for (int m = 0; m < M; ++m) {
            vector<float> part(subdim);         //Store each r_i
            int base = m * subdim;              //Which index of the points coordinates we start at for r_i
            for (int t = 0; t < subdim; ++t)
                part[t] = r[base + t];
            all_res_parts.push_back(move(part));
        }
        
        const auto& bucket = lists[cent_idxs[i]];
        for (const auto& e : bucket) {
            double dist = 0.0;
            for (int m = 0; m < M; ++m) {
                unsigned int code = e.codes[m];
                if(LUT[m][code] < 0.0){
                    LUT[m][code] = lp_dist(all_res_parts[m].begin(), all_res_parts[m].end(), subspace_centroids[m][code].begin(), 2.0);
                }
                dist += LUT[m][code];
            }


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