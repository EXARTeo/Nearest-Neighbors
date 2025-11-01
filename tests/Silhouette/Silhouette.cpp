#include "Silhouette.hpp"
#include "../../Common/Vector_distance.hpp"

using namespace std;

template <class T>
double compute_silhouette(const vector<vector<T>>& X, const vector<int>& labels, const vector<vector<T>>& centroids){
    size_t n = X.size();
    vector<double> s(n, 0.0);

    for (size_t i = 0; i < n; ++i) {
        int ci = labels[i];
        const auto& xi = X[i];

        //a(i): average dist of the "best" cluster 
        double a = 0.0;
        int countA = 0;
        for (size_t j = 0; j < n; ++j) {
            if (labels[j] == ci && i != j) {
                a += lp_dist(xi.begin(), xi.end(), X[j].begin(), 2.0);
                countA++;
            }
        }
        if (countA > 0)
            a /= countA;

        //b(i): average dist of the second "best" cluster 
        double minb = numeric_limits<double>::max();
        size_t b_inx = -1;
        for (size_t c = 0; c < centroids.size(); ++c) {
            if ((int)c == ci)
                continue;
            double temp = lp_dist(xi.begin(), xi.end(), centroids[c].begin(), 2.0);
            if (minb > temp){
                minb = temp;
                b_inx = c;
            }
        }

        double b = 0.0;
        int countB = 0;
        for (size_t j = 0; j < n; ++j) {
            if (labels[j] == (int)b_inx) {
                b += lp_dist(xi.begin(), xi.end(), X[j].begin(), 2.0);
                countB++;
            }
        }
        if (countB > 0)
            b /= countB;

        s[i] = (b - a) / max(a, b);
    }

    //Return the final silhouette
    double avg = 0.0;
    for (double si : s) avg += si;
    return avg / n;
}

// Explicit instantiation
template double compute_silhouette<unsigned char>(
    const vector<vector<unsigned char>>&,
    const vector<int>&,
    const vector<vector<unsigned char>>&);

template double compute_silhouette<float>(
    const vector<vector<float>>&,
    const vector<int>&,
    const vector<vector<float>>&);
