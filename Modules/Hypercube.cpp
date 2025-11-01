#include <unordered_set>
#include <queue>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <functional>

#include "../Includes/Hypercube.hpp"
#include "../Common/Vector_distance.hpp"

using namespace std;

// -----------------------------
//  F_Function Implementation
// -----------------------------

F_Function::F_Function(size_t dim, uint32_t kproj, double w, uint32_t table_size, uint32_t seed)
    : h(), f(), table_size(table_size){

    if (kproj <= 0) throw std::invalid_argument("kproj must be > 0");
    if (w <= 0.0)   throw std::invalid_argument("w must be > 0");

    h.reserve(kproj);
    f.reserve(kproj);

    for (uint32_t i = 0; i < kproj; ++i) {
        //Unique seeds for each h_i and f_i
        h.emplace_back(dim, w, seed + i * 101);
        f.emplace_back(seed + i * 201);
    }
}

std::vector<int> F_Function::operator()(const std::vector<double>& p) const {
    std::vector<int> bits;
    bits.reserve(h.size());     //kproj

    for (size_t i = 0; i < h.size(); ++i) {
        int h_val = h[i](p);    //Compute h_i(p)
        int bit = f[i](h_val);  //Compute f_i(h_i(p))
        bits.push_back(bit);
    }

    return bits;
}

int bits_to_int(const std::vector<int>& bits) {
    int result = 0;
    for (int bit : bits) {
        result = (result << 1) | bit;  //shift result left, add current bit
    }
    return result;
}

// -----------------------------
//  Hypercube Implementation
// -----------------------------

template <class T>
Hypercube<T>::Hypercube(size_t dim, int kproj, int M, int probes, double w, uint32_t table_size, uint32_t seed)
    : d(dim), kproj(kproj), M(M), probes(probes), w(w), table_size(table_size), f(dim, kproj, w, table_size, seed), table() /*initialize table*/ {

    if (kproj <= 0)  throw invalid_argument("kproj must be positive");
    if (M <= 0)      throw invalid_argument("M must be positive");
    if (probes <= 0) throw invalid_argument("probes must be positive");
    if (w <= 0)      throw invalid_argument("w must be positive");

    table.buckets.resize(table_size);
}

template <class T>
void Hypercube<T>::insert_object(uint32_t obj_id, const vector<T>& x) {
    vector<double> p(x.begin(), x.end());
    uint32_t bucket_idx = static_cast<uint32_t>(bits_to_int(f(p)));
    Entry<T> new_entry{obj_id, &x, bucket_idx};
    table.buckets[bucket_idx].push_back(new_entry);
}

template <class T>
void Hypercube<T>::build(const vector<vector<T>>& X) {
    for (uint32_t id = 0; id < X.size(); ++id)
        insert_object(id, X[id]);
}

//Returns a vector containing the all of q_id's neighbors' bucket indexes..
//..of hamming distance ham
vector<uint32_t> hamming_neighbors(const vector<int>& q_id, int kproj, int ham){
    vector<uint32_t> neighbors;

    if (ham == 0) {
        neighbors.push_back(bits_to_int(q_id));
        return neighbors;
    }

    vector<int> indices(kproj);
    iota(indices.begin(), indices.end(), 0); //[0, 1, 2, ..., kproj-1]

    vector<int> comb(ham);                   //contains the indexes of q_id on which we should flip the bits

    //Recursively create all neighbor combinations of the given hamming distance
    function<void(int,int)> gen = [&](int start, int k) {
        if (k == 0) {
            vector<int> neighbor_bits = q_id;
            for (int pos : comb)
                neighbor_bits[pos] = 1 - neighbor_bits[pos]; //flip bits
            neighbors.push_back(bits_to_int(neighbor_bits));
            return;
        }
        for (int i = start; i <= kproj - k; ++i) {
            comb[ham - k] = i;
            gen(i + 1, k - 1);
        }
    };

    gen(0, ham);
    return neighbors;
}

// -----------------------------
//  k-NN Query
// -----------------------------

template <class T>
vector<pair<uint32_t, double>> Hypercube<T>::query_knn(const vector<T>& q, int N) const {

    vector<double> qd(q.begin(), q.end());
    unordered_set<uint32_t> seen;
    priority_queue<pair<double, uint32_t>> max_heap; //(dist, id)

    vector<int> q_id = f(qd);   //get vector with bits, eg. [0,0,1,1,0]

    int checked_buckets = 0;    //max 'probes' buckets can be checked
    int checked_points = 0;     //max 'M' points can be searched

    //Expand by increasing Hamming distance
    for (int ham = 0; ham <= static_cast<int>(q_id.size()) && checked_buckets < probes && checked_points < M; ++ham){
        vector<uint32_t> neighbors = hamming_neighbors(q_id, q_id.size(), ham);

        for(int j = 0 ; j < static_cast<int>(neighbors.size()) && checked_points < M ; ++j){
            if (checked_buckets >= probes){
                break;
            }
            const auto& bucket = table.buckets[neighbors[j]];
            for (const auto& e : bucket) {
                if (checked_points >= M){
                    break;
                }
                if (seen.insert(e.obj_id).second) {
                    double dist = lp_dist(e.x->begin(), e.x->end(), q.begin(), 2.0); //L2
                    if ((int)max_heap.size() < N)
                        max_heap.emplace(dist, e.obj_id);
                    else if (dist < max_heap.top().first) {
                        max_heap.pop();
                        max_heap.emplace(dist, e.obj_id);
                    }
                    checked_points++;
                }
            }
            checked_buckets++;
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
vector<uint32_t> Hypercube<T>::query_range(const vector<T>& q, double R, size_t max_checked) const {

    vector<double> qd(q.begin(), q.end());
    unordered_set<uint32_t> seen;
    vector<uint32_t> res;

    vector<int> q_id = f(qd);   //get vector with bits, eg. [0,0,1,1,0]

    int checked_buckets = 0;    //max 'probes' buckets can be checked
    int checked_points = 0;     //max 'M' points can be searched

    //Expand by increasing Hamming distance
    for (int ham = 0; ham <= static_cast<int>(q_id.size()) && checked_buckets < probes && checked_points < M; ++ham) {
        vector<uint32_t> neighbors = hamming_neighbors(q_id, q_id.size(), ham);

        for (int j = 0; j < static_cast<int>(neighbors.size()) && checked_points < M; ++j) {
            if (checked_buckets >= probes){
                break;
            }
            const auto& bucket = table.buckets[neighbors[j]];
            for (const auto& e : bucket) {
                if (checked_points >= M){
                    break;
                }
                if (seen.insert(e.obj_id).second) {
                    double dist = lp_dist(e.x->begin(), e.x->end(), q.begin(), 2.0); // L2
                    if (dist <= R){
                        res.push_back(e.obj_id);
                    }
                    checked_points++;
                }
            }
            checked_buckets++;
        }
    }
    return res;
}