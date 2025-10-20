#pragma once
#include <cstdint>
#include <vector>
#include <algorithm>
#include <utility>

#include "Vector_distance.hpp"


template <class T>
std::vector<std::pair<uint32_t,double>> brute_force(std::vector<T> &q, std::vector<std::vector<T>> &D){

    std::vector<std::pair<uint32_t, double>> v;
    if (D.empty()) return v;

    v.reserve(D.size());
    for (uint32_t id = 0; id < static_cast<uint32_t>(D.size()); ++id) {
        double dist = lp_dist(q.begin(), q.end(), D[id].begin(), 2.0);  //L2
        v.emplace_back(id, dist);
    }

    std::sort(v.begin(), v.end(), [](const auto& a, const auto& b){ return a.second < b.second; });

    return v;
}