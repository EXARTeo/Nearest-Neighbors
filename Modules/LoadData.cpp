#include "../Includes/LoadData.hpp"
#include <iostream>
#include <fstream>


using namespace std;

static uint32_t read_u32_be(ifstream& f) {
    uint8_t b[4];
    f.read((char*)b, 4);
    return (uint32_t(b[0])<<24) | (uint32_t(b[1])<<16) | (uint32_t(b[2])<<8) | uint32_t(b[3]);
}

DataImages<uint8_t> load_mnist(const string& path) {
    ifstream f(path, ios::binary);

    if(!f) throw runtime_error("open failed");

    uint32_t magic = read_u32_be(f);
    if(magic != 2051) throw runtime_error("bad magic (want 2051)");

    uint32_t n    = read_u32_be(f);
    uint32_t rows = read_u32_be(f);
    uint32_t cols = read_u32_be(f);

    uint32_t d = rows * cols;
    vector<vector<uint8_t>> X(n, vector<uint8_t>(d));

    for(uint32_t i = 0; i < n; i++)
        f.read((char*)X[i].data(), d);

    return {n, d, move(X)};
}



DataImages<float> load_sift(const string& path) {
    ifstream f(path, ios::binary);

    if(!f) throw runtime_error("open failed");

    vector<vector <float>> X;
    uint32_t dim = 0;
    uint32_t n = 0;

    while(true){
        int32_t d;

        if(!f.read((char*)&d, 4)) break; //EOF

        if(dim == 0){
            dim = d;
        }
        else if(d!=dim){
            throw runtime_error("mixed dims in sift");
        }
        vector<float> v(dim);
        f.read((char*)v.data(), sizeof(float)*dim);
        if(!f) throw runtime_error("truncated sift");

        X.push_back(move(v));
        n++;
    }
    if(dim==0) throw runtime_error("empty sift");

    return {n, dim, move(X)};
}