#pragma once
#include <vector>

class Hhash {
private:
    std::vector<double> v;
    double t;
    double w;

public:
    explicit Hhash(size_t d, double width, int seed = 1);

    //Overload function for "h(p)" with <vector> input
    int operator()(const std::vector<double>& p) const;

    //Overload function for "h(p)" with array(pointer) input
    int operator()(const double* p, size_t d) const;
};
