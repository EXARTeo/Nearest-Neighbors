#include <vector>
#include <random>
#include <numeric>
#include <cmath>
#include <stdexcept>
#include <string>

#include "../Includes/Fhash.hpp"

using namespace std;


Fhash::Fhash(int seed) : seed(seed) {}

//Overload function for "f(h_value)"
bool Fhash::operator()(int h_value) const {

    //If we have already computed the value of f(h_value), return it
    auto it = f_values.find(h_value);
    if (it != f_values.end()){
        return it->second;
    }

    //Otherwise assign a new random bit
    std::mt19937_64 rng(static_cast<uint64_t>((h_value)^seed));
    std::uniform_int_distribution<int> bit_dist(0, 1);
    bool bit = static_cast<bool>(bit_dist(rng));

    //Store the result (0 or 1) to the map
    f_values[h_value] = bit;

    return bit;
}