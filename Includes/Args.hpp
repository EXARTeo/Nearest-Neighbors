#pragma once
#include <string>
#include <stdexcept>
#include <algorithm>
#include <optional>

enum class Mode {
    Unknown = 0,
    LSH,
    Hypercube,
    IVFFlat,
    IVFPQ
};

inline const char* to_string(Mode algo) {
    switch (algo) {
        case Mode::LSH:       return "LSH";
        case Mode::Hypercube: return "Hypercube";
        case Mode::IVFFlat:   return "IVFFlat";
        case Mode::IVFPQ:     return "IVFPQ";
        case Mode::Unknown:   return "Unknown";
    }
    return "UNKNOWN";
}

enum class Type {
    Unknown = 0,
    MNIST,
    SIFT
};

struct Args {
    //Common
    Mode mode = Mode::Unknown;          //default value unknown
    std::string input_file;             //-d
    std::string query_file;             //-q
    std::string output_file;            //-o
    Type dataset_type = Type::Unknown;  //-type (default value unknown)
    
    int N = 1;                          //-N
    int seed = 1;                       //-seed
    bool range = true;                  //-range (true|false)
    double R = -1.0;                    //-R     (2000|2)

    //LSH
    int k = 4;                          //-k
    int L = 5;                          //-L
    double w = 4.0;                     //-w (Hypercube too)

    //Hypercube
    int kproj = 14;                     //-kproj
    int M = 10;                         //-M
    int probes = 2;                     //-probes

    //IVFFlat / IVFPQ
    int kclusters = 50;                 //-kclusters
    int nprobe = 5;                     //-nprobe

    //IVFPQ only
    int nbits = 8;                      //-nbits
    int M_pq = 16;                      //-M
};

//Read through the user input and saves the values
Args load_args(int argc, char** argv);