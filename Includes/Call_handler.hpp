#pragma once
#include "Args.hpp"
#include "LoadData.hpp"

#include "LSH.hpp"

#include "../Common/Search_dispatcher.hpp"


template<class T>
void call_dispatcher(Args& args, DataImages<T>& D, DataImages<T>& Q) {
    switch (args.mode) {
        case Mode::LSH:{
            LSH<T> table(D.dim, args.k, args.L, args.w, D.n/4, args.seed);
            table.build(D.X);
            searcher<T>(args, D.X, Q.X,
                [&table](const vector<T>& q, int N) {return table.query_knn(q, N);}, 
                [&table](const vector<T>& q, double R) {return table.query_range(q, R);});
            break;
        }
        case Mode::Hypercube:
        case Mode::IVFFlat:
        case Mode::IVFPQ:
        case Mode::Unknown:
        default:
            throw std::runtime_error("No mode selected. Use -lsh / -hypercube / -ivfflat / -ivfpq");
    }
}