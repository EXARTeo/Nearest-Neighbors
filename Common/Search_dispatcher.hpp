//TESTING FILE
//SAVING OUR TIME BY PRELOADED BFS TO ALL QUERIES

#pragma once

#include <fstream>
#include <iostream>
#include <vector>
#include <chrono>
#include <unordered_set>
#include <iomanip>
#include <limits>

#include "../tests/BF_results/BFloader.hpp"
#include "../Includes/Args.hpp"

using namespace std;
using namespace std::chrono;

template <class T, class Knn, class Range>
void searcher(Args& args, vector<vector<T>>& X, vector<vector<T>>& Q, Knn knn, Range range) {
    ofstream out(args.output_file);
    if (!out) {
        cerr << "Failed to open output file: " << args.output_file << "\n";
        return;
    }

    out << to_string(args.mode)<<"\n\n";

    int N = args.N;
    size_t Qn = Q.size();

    double sum_AF       = 0.0;
    double sum_Recall   = 0.0;
    double sum_tApprox  = 0.0;

    for (size_t qi = 0; qi < Qn; qi++) {
        auto& q = Q[qi];

        //Approximate Knn
        auto Tknn0 = high_resolution_clock::now();
        vector<pair<uint32_t,double>> Knn_approx = knn(q, N);
        auto Tknn1 = high_resolution_clock::now();
        double Tknn = duration<double>(Tknn1 - Tknn0).count();
        sum_tApprox += Tknn;

        //Brute-force
        string BFpreload;
        if (args.dataset_type == Type::MNIST)
            BFpreload = "tests/BF_results/BFmnist/BFmnist" + to_string(qi + 1) + ".txt";
        else
            BFpreload = "tests/BF_results/BFsift/BFsift" + to_string(qi + 1) + ".txt";

        vector<pair<uint32_t,double>> true_dist = load_bf(BFpreload);

        if (!Knn_approx.empty())
            sum_AF += Knn_approx[0].second / true_dist[0].second;

        //Query prints
        out << "Query: " << qi + 1 << '\n';
        for (size_t i = 0; i < Knn_approx.size(); i++){
            out <<"Nearest neighbor-"<<i + 1<<": "<<Knn_approx[i].first<<'\n';
            out <<"distanceApproximate: "         <<Knn_approx[i].second<<'\n';
            out <<"distanceTrue: "                <<true_dist[i].second<<'\n';
        }

        int hits = 0;
        for (size_t i = 0; i < Knn_approx.size(); i++){
            for(size_t j = 0; j < true_dist.size(); j++){
                if(Knn_approx[i].first == true_dist[j].first)
                    hits++;
            }
        }

        double recall = double(hits)/double(true_dist.size());
        sum_Recall += recall;

        //Approximate range
        if(args.range){
            out <<"\nR-near neighbors:\n";
            vector<uint32_t> range_approx = range(q, args.R, 0);
            for(size_t i = 0; i < range_approx.size(); i++){
                out << range_approx[i]<<'\n';
            }
        }
        out <<'\n';
    }

    double bf_avtime = load_bf_time(static_cast<int>(args.dataset_type));

    out <<'\n';
    out << "Average AF: "            << sum_AF/Qn               << "\n";
    out << "Recall@N: "              << sum_Recall/Qn           << "\n";
    out << "QPS: "                   << Qn/sum_tApprox          << "\n";    //TODO : (Qn * N)/sum_tApprox
    out << "tApproximateAverage: "   << sum_tApprox/Qn          << "\n";
    out << "tTrueAverage: "          << bf_avtime               << "\n";

}
