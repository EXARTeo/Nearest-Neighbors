#pragma once
#include <vector>
#include <cstdint>
#include <limits>
#include <random>
#include <utility>

#include "../Common/HashTable.hpp"

template <class T>
class IVFFlat {
public:
    IVFFlat(int kclusters, int nprobe, uint32_t seed = 1);

	//Insert an object to the table
	void insert_object(uint32_t obj_id, const std::vector<T>& x, const std::vector<std::vector<T>>& centroids);

	//Initialize this table
	void build(const std::vector<std::vector<T>>& X);

    //k-NN : returns (obj_id, distance)
    std::vector<std::pair<uint32_t, double>> query_knn(const std::vector<T>& q, int N) const;

    //Range search: returns all ids with |x - q| < R
    std::vector<uint32_t> query_range(const std::vector<T>& q, double R, size_t max_checked = 0) const;     //max checked is useless right now

private:
    uint32_t seed;
    int kclusters;                                  //number of clusters, aka number of 'buckets'
    int nprobe;                                     //number of nearest clusters searched
    std::vector<std::vector<T>> final_centroids;    //vector containing the finalized centroids
    HashTable<T> table;
};


template class IVFFlat<uint8_t>;
template class IVFFlat<float>;