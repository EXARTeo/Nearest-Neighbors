#pragma once

#include <fstream>
#include <iostream>
#include <vector>
#include <chrono>
#include <unordered_set>
#include <iomanip>
#include <limits>

#include "Brute_force.hpp"
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
    double sum_tTrue    = 0.0;

    for (size_t qi = 0; qi < Qn; qi++) {
        auto& q = Q[qi];

        //Approximate Knn
        auto Tknn0 = high_resolution_clock::now();
        vector<pair<uint32_t,double>> Knn_approx = knn(q, N);
        auto Tknn1 = high_resolution_clock::now();
        double Tknn = duration<double>(Tknn1 - Tknn0).count();
        sum_tApprox += Tknn;

        //Brute-force
        auto Tbf0 = high_resolution_clock::now();
        vector<pair<uint32_t,double>> true_dist = brute_force(q, X);
        auto Tbf1 = high_resolution_clock::now();
        double Tbf = duration<double>(Tbf1 - Tbf0).count();
        sum_tTrue += Tbf;

        N = (N > (int)true_dist.size()) ? true_dist.size() : N;

        sum_AF += Knn_approx[0].second / true_dist[0].second;

        //Query prints
        out << "Query: " << qi + 1 << "\n";
        for (int i = 0; i < N; i++){
            out <<"Nearest neighbor-"<<i + 1<<": "<<Knn_approx[i].first<<'\n';
            out <<"distanceApproximate: "        <<Knn_approx[i].second<<'\n';
            out <<"distanceTrue: "               <<true_dist[i].second<<'\n';
        }

        int hits = 0;
        for (int i = 0; i < N; i++){
            for(int j = 0; j < N; j++){
                if(Knn_approx[i].first == true_dist[j].first)
                    hits++;
            }
        }
        double recall = double(hits)/double(N);
        sum_Recall += recall;

        //Approximate range
        if(args.range){
            out <<"\nR-near neighbors:\n";
            vector<uint32_t> range_approx = range(q, args.R);
            for(size_t i = 0; i < range_approx.size(); i++){
                out << range_approx[i]<<'\n';
            }
        }
        out <<'\n';
    }
    out <<'\n';
    out << "Average AF: "            << sum_AF/Qn               << "\n";
    out << "Recall@N: "              << sum_Recall/Qn           << "\n";
    out << "QPS: "                   << Qn/sum_tApprox          << "\n";    //TODO : (Qn * N)/sum_tApprox
    out << "tApproximateAverage: "   << sum_tApprox/Qn          << "\n";
    out << "tTrueAverage: "          << sum_tTrue/Qn            << "\n";
    out << "\n";

}
