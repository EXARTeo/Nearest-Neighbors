#pragma once
#include <unordered_map>
#include <random>
#include <cstdint>

class Fhash {
private:
    int seed;
    mutable std::unordered_map<int, bool> f_values;      //store already computed f_values

public:
    explicit Fhash(int seed = 1);

    //Overload function for "f(h_value)"
    bool operator()(int h_value) const;
};