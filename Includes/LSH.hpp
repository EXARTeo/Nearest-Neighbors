#pragma once
#include <vector>
#include <cstdint>
#include <limits>
#include <random>
#include <utility>

#include "../Common/HashTable.hpp"
#include "Hhash.hpp"


//AKA   M_Mod=2^32 - 5 
#define M_Mod 4294967291u


//Amplified hash g: combines k h_i with r_i (and mod M) -> (bucket_index, lsh_id)
struct GFunction {
    std::vector<Hhash> h;         //h_i (i e[1, k])
    std::vector<uint32_t> r;      //r_i (random nums i e[1, k])
    uint32_t M;                   //AKA M_Mod
    uint32_t table_size;          //AKA buckets of this table

	GFunction(size_t dim, uint32_t k, double w, uint32_t table_size, uint32_t M, uint32_t seed);

    //ID(p) = (Σ r_i * h_i(p)) mod M
    uint32_t ID(const std::vector<double>& p) const;

    //Index bucket: g(p) = ID(p) mod table_size
    uint32_t gfunc(const std::vector<double>& p) const;
};

//LSH index: L tables, each one with diffrent g
template <class T>
class LSH {
public:
    LSH(size_t dim, uint32_t k, uint32_t L, double w, uint32_t table_size, uint32_t seed = 1, uint32_t M = M_Mod);

	//Insert an object to the table
	void insert_object(uint32_t obj_id, const std::vector<T>& x);

	//Initialize this LSH table
	void build(const std::vector<std::vector<T>>& X);

    //k-NN : returns (obj_id, distance)
    std::vector<std::pair<uint32_t, double>> query_knn(const std::vector<T>& q, int N) const;

    //Range search: returns all ids with |x - q| < R
    std::vector<uint32_t> query_range(const std::vector<T>& q, double R, size_t max_checked = 0) const;

private:
    size_t d;
    uint32_t k;
	uint32_t L;
    double w;
    uint32_t table_size;
	uint32_t M;
    std::vector<GFunction> g;			//L diffrent g
    std::vector<HashTable<T>> tables;	//L tables
};


template class LSH<uint8_t>;
template class LSH<float>;