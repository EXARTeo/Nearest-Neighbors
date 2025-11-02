#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <string>


std::vector<std::pair<uint32_t,double>> load_bf(std::string& preloaded_file, int N){
    std::vector<std::pair<uint32_t,double>> results;
    std::ifstream in(preloaded_file);
    if (!in)
        return results;

    uint32_t id;
    double dist;
    while (in >> id >> dist && N != 0){
        N--;
        results.emplace_back(id, dist);
    }
    return results;
}

double load_bf_time(int flag){
    std::ifstream in("tests/BF_results/BFtimes.txt");   //relevant path
    if (!in) return -1.0;

    double t_mnist = -1.0, t_sift = -1.0;
    in >> t_mnist >> t_sift;
    
    if (flag == 1) return t_mnist;
    if (flag == 2) return t_sift;
    return -1.0;
}