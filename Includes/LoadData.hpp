#pragma once
#include <string>
#include <vector>
#include <cstdint>


template <class D>
struct DataImages {
    uint32_t n;
    uint32_t dim;
    //uint8_t (MNIST) or float (SIFT)
    std::vector<std::vector<D>> X; //n x dim
};

DataImages<uint8_t> load_mnist(const std::string& path);

DataImages<float> load_sift(const std::string& path);
