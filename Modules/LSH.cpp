#include <unordered_set>
#include <queue>
#include <algorithm>
#include <cmath>
#include <iostream>

#include "../Includes/LSH.hpp"
#include "../Common/Vector_distance.hpp"

using namespace std;

// -----------------------------
//  GFunction Implementation
// -----------------------------

GFunction::GFunction(size_t dim, uint32_t k, double w, uint32_t table_size, uint32_t M, uint32_t seed)
    : h(), r(k), M(M), table_size(table_size)
{
    mt19937_64 rng(seed);
    uniform_int_distribution<uint32_t> uni_r(1, M - 1);
    for (uint32_t i = 0; i < k; ++i) r[i] = uni_r(rng);

    h.reserve(k);

    for (uint32_t i = 0; i < k; ++i)
        h.emplace_back(dim, w, seed * (i + 1));
}

uint32_t GFunction::ID(const vector<double>& p) const {
    uint64_t acc = 0;
    for (uint32_t i = 0; i < h.size(); ++i) {
        uint64_t hi = h[i](p);
        uint64_t ri = r[i];
        acc = ((acc % M) + (((ri % M) * (hi % M)) % M)) % M;
    }
    return static_cast<uint32_t>(acc);
}

uint32_t GFunction::gfunc(const vector<double>& p) const {
    return ID(p) % table_size;
}

// -----------------------------
//  LSH Implementation
// -----------------------------

template <class T>
LSH<T>::LSH(size_t dim, uint32_t k, uint32_t L, double w, uint32_t table_size, uint32_t seed, uint32_t M)
    : d(dim), k(k), L(L), w(w), table_size(table_size), M(M) {
    
    //L diffrent g functios
    g.reserve(L);
    for (uint32_t i = 0; i < L; ++i)
        g.emplace_back(dim, k, w, table_size, M, seed * (i + 1) + i);

    //L diffrent tables
    tables.resize(L);
    for (auto& ht : tables)
        ht.buckets.resize(table_size);
}

template <class T>
void LSH<T>::insert_object(uint32_t obj_id, const vector<T>& x) {

    vector<double> p(x.begin(), x.end());
    for (uint32_t i = 0; i < L; ++i) {
        uint32_t lsh_id = g[i].ID(p);
        uint32_t bucket_idx = lsh_id % table_size;  //g[i].gfunc(p);
        Entry<T> new_enrty{obj_id, &x, lsh_id};
        tables[i].buckets[bucket_idx].push_back(new_enrty);
    }
}

template <class T>
void LSH<T>::build(const vector<vector<T>>& X) {
    for (uint32_t id = 0; id < X.size(); ++id)
        insert_object(id, X[id]);

    // for (uint32_t i = 0; i < L; i++){
    //     cout <<"[DEBUG] G_"<<i<<endl;
    //     for (uint32_t j = 0; j < table_size; j++){
    //         cout <<"[DEBUG] bucket_"<<j<<" has size : "<<tables[i].buckets[j].size()<<endl;
    //     }
    // }
}


// -----------------------------
//  k-NN Query
// -----------------------------

template <class T>
vector<pair<uint32_t, double>> LSH<T>::query_knn(const vector<T>& q, int N) const {

    vector<double> qd(q.begin(), q.end());
    unordered_set<uint32_t> seen;
    priority_queue<pair<double, uint32_t>> max_heap; //(dist, id)
    // cout <<"[DEBUG] STARTING KNN for query with the ID:"<<endl;
    for (uint32_t i = 0; i < L; ++i) {
        // cout <<"[DEBUG] ID : "<<g[i].ID(qd)<<endl;
 
        uint32_t q_id = g[i].ID(qd);
        uint32_t bucket_idx = q_id % table_size;
        // cout <<'\n'<<"[DEBUG] q_id :  "     <<q_id<<endl; 
        // cout <<'\n'<<"[DEBUG] bucket_idx :  "<<bucket_idx<<endl;

        const auto& bucket = tables[i].buckets[bucket_idx];
        // cout <<"[DEBUG] Size of bucket : "<< bucket.size()<<endl;
        for (const auto& e : bucket) {
            // cout <<"[DEBUG] obj_id: " <<static_cast<uint32_t>(e.obj_id) <<endl;
            // cout <<"[DEBUG] func_id: " <<static_cast<uint32_t>(e.func_id) <<endl;

            if (seen.insert(e.obj_id).second) {
                if (q_id == e.func_id){
                    double dist = lp_dist(e.x->begin(), e.x->end(), q.begin(), 2.0);    //L2
                    if ((int)max_heap.size() < N)
                        max_heap.emplace(dist, e.obj_id);
                    else if (dist < max_heap.top().first) {
                        max_heap.pop();
                        max_heap.emplace(dist, e.obj_id);
                    }
                }
            }
        }
    }

    //TODO OPTIMIZE
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
vector<uint32_t> LSH<T>::query_range(const vector<T>& q, double R, size_t max_checked) const {
    vector<double> qd(q.begin(), q.end());
    unordered_set<uint32_t> results;
    size_t checked = 0;
    // cout <<"[DEBUG] STARTING range for query with the ID:"<<endl;
    for (uint32_t i = 0; i < L; ++i) {
        // cout <<"[DEBUG] ID : "<<g[i].ID(qd)<<endl;
        uint32_t q_id = g[i].ID(qd);
        uint32_t bucket_idx = q_id % table_size;
        // cout <<'\n'<<"[DEBUG] bucket_idx :  "<<bucket_idx<<endl;

        const auto& bucket = tables[i].buckets[bucket_idx];
        for (const auto& e : bucket) {
            // cout <<"[DEBUG] obj_id: " <<static_cast<uint32_t>(e.obj_id) <<endl;
            if (results.find(e.obj_id) == results.end()) {
                if (q_id == e.func_id){
                    double dist = lp_dist(e.x->begin(), e.x->end(), q.begin(), 2.0);    //L2
                    ++checked;
                    if (dist < R){
                        results.insert(e.obj_id);
                    }
                    if (max_checked && checked >= max_checked)
                        break;
                }
            }
        }
        if (max_checked && checked >= max_checked)
            break;
    }

    return vector<uint32_t>(results.begin(), results.end());
}

// Maybe used for safety maybe not ¯\_(ツ)_/¯
// template class LSH<uint8_t>;
// template class LSH<float>;
// template class LSH<double>;
