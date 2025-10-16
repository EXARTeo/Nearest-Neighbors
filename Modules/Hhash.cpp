#include <vector>
#include <random>
#include <numeric>
#include <cmath>
#include <stdexcept>
#include <string>

#include "../Includes/Hhash.hpp"

using namespace std;


Hhash::Hhash(size_t d, double width, int seed) : v(d), w(width) {
    if (w <= 0.0) {
        throw invalid_argument("Hhash: w must be > 0");
    }
    mt19937_64 rng(static_cast<uint64_t>(seed));
    normal_distribution<double> normal(0.0, 1.0);
    uniform_real_distribution<double> uni(0.0, w);

    //v ~ N(0,1)^d
    for (double &xi : v)
        xi = normal(rng);

    //t ~ U[0, w)
    t = uni(rng);
}

//Overload function for "h(p)" with <vector> input
int Hhash::operator()(const vector<double>& p) const {
    if (p.size() != v.size())
        throw invalid_argument("Hhash: dimension mismatch");

    const double pv = inner_product(p.begin(), p.end(), v.begin(), 0.0);
    const double value = floor((pv + t) / w);
    return static_cast<int>(value);
}

//Overload function for "h(p)" with array(pointer) input
int Hhash::operator()(const double* p, size_t d) const {
    if (d != v.size())
        throw invalid_argument("Hhash: dimension mismatch");

    double pv = 0.0;
    for (size_t i = 0; i < d; ++i)
        pv += p[i] * v[i];
    const double value = floor((pv + t) / w);
    return static_cast<int>(value);
}
