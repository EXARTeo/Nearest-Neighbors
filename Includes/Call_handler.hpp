#include "Args.hpp"
#include "vector"


template<class T>
void call_dispatcher(const Args& args, const std::vector<std::vector<T>>& X, const std::vector<std::vector<T>>& Q, int dim) {
    switch (args.mode) {
        case Mode::LSH:
            LSH_search(args, X, Q, dim);
            break;
        case Mode::Hypercube:
        case Mode::IVFFlat:
        case Mode::IVFPQ:
        default:
            throw std::runtime_error("No mode selected. Use -lsh / -hypercube / -ivfflat / -ivfpq");
    }
}