#pragma once
#include <vector>
#include <cstdint>
#include <limits>
#include <random>
#include <utility>

template <class T>
class IVFPQ {
public:
    IVFPQ(size_t d, int kclusters, int nprobe, int M, int nbits, uint32_t seed = 1);

    //Insert an object to the table
    void insert_object(uint32_t obj_id, const std::vector<T>& x);

    //Initialize this table
    void build(const std::vector<std::vector<T>>& X);

    //k-NN : returns (obj_id, distance)
    std::vector<std::pair<uint32_t, double>> query_knn(const std::vector<T>& q, int N) const;

    // //Range search: returns all ids with |x - q| < R
    std::vector<uint32_t> query_range(const std::vector<T>& q, double R, size_t max_checked = 0) const;     //max checked is useless right now

private:
    uint32_t seed;
    int kclusters;      //Number of clusters, aka number of 'buckets'
    int nprobe;         //Number of nearest clusters searched
    int M;              //ri/M
    int nbits;          //-nbits
    int s;              //2^nbits

    size_t d = -1;      //Dimention of the datasets' points
    int subdim = -1;    //== d / M


    struct PQEntry {
        uint32_t obj_id;
        std::vector<unsigned int> codes;    //Size M, each code e[0, s-1]
        uint32_t list_id;                   //Coarse list id (centroid index)
    };

    std::vector<std::vector<std::vector<T>>> residuals;             //residuals[M] (AKA r(x) = x - c(x))
    std::vector<std::vector<T>> final_centroids;                    //The finalized centroids
    std::vector<std::vector<std::vector<T>>> subspace_centroids;    //Subspace_centroids[M][s] (AKA C[M][h])
    std::vector<std::vector<PQEntry>> lists;                        //Inverted lists: each coarse list saves PQ codes

    void build_subcentroids(const std::vector<std::vector<T>>& X);
    void encode_residual(const std::vector<std::vector<T>>& r, std::vector<unsigned int>& codes) const;
    void compute_residual(const std::vector<T>& x, int j, std::vector<T>& r) const;
    std::pair<double, int> nearest_centroid(const std::vector<T>& x, const std::vector<std::vector<T>>& centroids);
    std::vector<std::vector<T>> get_random_subset(const std::vector<std::vector<T>>& X, int cluster_num);
    std::vector<std::vector<T>> lloyds_alg(const std::vector<std::vector<T>>& X, std::vector<int>& centroid_idxs, int kclusters, uint32_t seed);
};


template class IVFPQ<uint8_t>;
template class IVFPQ<float>;