#pragma once
#include <cmath>
#include <iterator>
#include <stdexcept>


template <class ItX, class ItY>
double lp_dist(ItX x_start, ItX x_end, ItY y_start, double k){
    if (k < 0.0) {
        throw std::invalid_argument("lp_dist: k must be >= 0");
    }

    //L_inf: max_i |x_i - y_i|
    if (std::isinf(k)) {
        long double max = 0.0L;
        for (; x_start != x_end; ++x_start, ++y_start) {
            long double dist = fabsl((long double)*x_start - (long double)*y_start);
            if (dist > max)
                max = dist;
        }
        return (double)max;
    }

    //L_0: #nonzero entries in v.
    if (k == 0.0) {
        long double cnt = 0.0L;
        for (; x_start != x_end; ++x_start, ++y_start) {
            if (*x_start != *y_start)
                cnt += 1.0L;
        }
        return (double)cnt;
    }

    long double dist = 0.0L;

    //L_2
    if (k == 2.0) {
        for (; x_start != x_end; ++x_start, ++y_start) {
            long double my_pow = (long double)(*x_start) - (long double)(*y_start);
            dist += my_pow * my_pow;
        }
        return (double)powl(dist, 1.0L / (long double)k);
    }

    //L_k: k > 0
    for (; x_start != x_end; ++x_start, ++y_start) {
        dist += powl(fabsl((long double)(*x_start) - (long double)(*y_start)), (long double)k);
    }

    if (k == 1.0) return (double)dist; //Saves some time =-)

    return (double)powl(dist, 1.0L / (long double)k);
}