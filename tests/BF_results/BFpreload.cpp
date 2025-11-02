//$ g++ -I../../Includes -I../../Common BFpreload.cpp ../../Modules/LoadData.cpp -o BFpreload
//$ ./BFpreload ../../imgs/train-images.idx3-ubyte ../../imgs/t10k-images.idx3-ubyte ../../imgs/sift_base.fvecs ../../imgs/sift_query.fvecs

#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <string>
#include <cstdlib>
#include <omp.h>

#include "../../Common/LoadData.hpp"
#include "../../Common/Brute_force.hpp"

using namespace std;
using namespace std::chrono;

template<class T>
void BF_writer(DataImages<T>& D, DataImages<T>& Q, const string& prefix, double& sum_secs) {

    for (size_t i = 0; i < Q.X.size(); ++i) {
        ofstream out(prefix + "/" + prefix + to_string(i+1) + ".txt");

        auto t0 = high_resolution_clock::now();
        vector<pair<uint32_t,double>> true_dist = brute_force(Q.X[i], D.X);
        auto t1 = high_resolution_clock::now();

        sum_secs += duration<double>(t1 - t0).count();
        for (size_t j = 0; j < true_dist.size(); ++j)
            out << true_dist[j].first << ' ' << true_dist[j].second << '\n';
    }
}

int main(int argc, char** argv){
    double sum_mnist = 0.0, sum_sift = 0.0;

    DataImages<uint8_t> Dm = load_mnist(argv[1]);
    DataImages<uint8_t> Qm = load_mnist(argv[2]);
    BF_writer<uint8_t>(Dm, Qm, "BFmnist", sum_mnist);

    DataImages<float> Ds = load_sift(argv[3]);
    DataImages<float> Qs = load_sift(argv[4]);
    BF_writer<float>(Ds, Qs, "BFsift", sum_sift);

    ofstream bftimes("BFtimes.txt");

    bftimes << sum_mnist/Qm.X.size() << '\n' << sum_sift/Qs.X.size() << '\n';
    return 0;
}
