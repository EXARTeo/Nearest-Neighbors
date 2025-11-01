#pragma once
#include <vector>
#include <cstdint>
#include <limits>
#include <random>
#include <utility>

#include "../Common/HashTable.hpp"

#include "Fhash.hpp"
#include "Hhash.hpp"

//Hash function f_i(h_i(p))
struct F_Function {
    std::vector<Hhash> h;         //h_i (i e[1, kproj])
    std::vector<Fhash> f;         //f_i (i e[1, kproj])
    uint32_t table_size;          //AKA buckets of this table

	F_Function(size_t dim, uint32_t kproj, double w, uint32_t table_size, uint32_t seed);

    std::vector<int> operator()(const std::vector<double>& p) const;
};

template <class T>
class Hypercube {
public:
    Hypercube(size_t dim, int kproj, int M, int probes, double w, uint32_t table_size, uint32_t seed = 1);

	//Insert an object to the table
	void insert_object(uint32_t obj_id, const std::vector<T>& x);

	//Initialize this table
	void build(const std::vector<std::vector<T>>& X);

    //k-NN : returns (obj_id, distance)
    std::vector<std::pair<uint32_t, double>> query_knn(const std::vector<T>& q, int N) const;

    //Range search: returns all ids with |x - q| < R
    std::vector<uint32_t> query_range(const std::vector<T>& q, double R, size_t max_checked = 0) const;     //max checked is useless right now

private:
    size_t d;
    int kproj;              //d'
    int M;                  //max number of neighbors searched
    int probes;             //max number of buckets searched
    double w;
    uint32_t table_size;
    F_Function f;           //contains all f_i and h_i functions
    HashTable<T> table;
};


template class Hypercube<uint8_t>;
template class Hypercube<float>;