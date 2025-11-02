#pragma once

#include "../Includes/LSH.hpp"
#include "../Includes/Hypercube.hpp"
#include "../Includes/IVFFlat.hpp"
#include "../Includes/IVFPQ.hpp"

#include "Search_dispatcher.hpp"
#include "LoadData.hpp"
#include "Args.hpp"


template<class T>
void call_dispatcher(Args& args, DataImages<T>& D, DataImages<T>& Q) {
    if (D.X.size() <= 0)
        throw runtime_error("The \"-d\" input dataset is empty");
    if (Q.X.size() <= 0)
        throw runtime_error("The \"-q\" query dataset is empty");
    if (D.dim != Q.dim)
        throw runtime_error("Input dataset has different dimention from query");

    switch (args.mode) {
        case Mode::LSH:{
            LSH<T> table(D.dim, args.k, args.L, args.w, D.n/4, args.seed);
            table.build(D.X);
            searcher<T>(args, D.X, Q.X,
                [&table](const vector<T>& q, int N) {return table.query_knn(q, N);}, 
                [&table](const vector<T>& q, double R, size_t max_checked) {return table.query_range(q, R, max_checked);});
            break;
        }
        case Mode::Hypercube:{
            Hypercube<T> table(D.dim, args.kproj, args.M, args.probes, args.w, static_cast<uint32_t>(1 << args.kproj), args.seed);
            table.build(D.X);
            searcher<T>(args, D.X, Q.X,
                [&table](const vector<T>& q, int N) {return table.query_knn(q, N);}, 
                [&table](const vector<T>& q, double R, size_t max_checked) {return table.query_range(q, R, max_checked);});
            break;
        }
        case Mode::IVFFlat:{
            IVFFlat<T> table(args.kclusters, args.nprobe, args.seed);
            table.build(D.X);
            searcher<T>(args, D.X, Q.X,
                [&table](const vector<T>& q, int N) {return table.query_knn(q, N);}, 
                [&table](const vector<T>& q, double R, size_t max_checked) {return table.query_range(q, R, max_checked);});
            break;
        }
        case Mode::IVFPQ:{
            IVFPQ<T> table(D.dim, args.kclusters, args.nprobe, args.M_pq, args.nbits, args.seed);
            table.build(D.X);
            searcher<T>(args, D.X, Q.X,
                [&table](const vector<T>& q, int N) {return table.query_knn(q, N);}, 
                [&table](const vector<T>& q, double R, size_t max_checked) {return table.query_range(q, R, max_checked);});
            break;
        }
        case Mode::Unknown:
        default:
            throw std::runtime_error("No mode selected. Use -lsh / -hypercube / -ivfflat / -ivfpq");
    }
}