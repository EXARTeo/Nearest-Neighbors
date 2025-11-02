#include <iostream>
#include <fstream>
#include <vector>
#include <limits>
#include <iomanip>

#include "Common/Search_dispatcher.hpp"
#include "Common/Vector_distance.hpp"
#include "Common/Call_handler.hpp"
#include "Common/LoadData.hpp"
#include "Common/Args.hpp"

using namespace std;

int main(int argc, char** argv) {

    Args args = load_args(argc, argv);

    //MNIST
    if (args.dataset_type == Type::MNIST) {
        DataImages<uint8_t> D = load_mnist(args.input_file);
        DataImages<uint8_t> Q = load_mnist(args.query_file);

        call_dispatcher<uint8_t>(args, D, Q);
    }
    //SIFT
    else if(args.dataset_type == Type::SIFT) {
        DataImages<float> D = load_sift(args.input_file);
        DataImages<float> Q = load_sift(args.query_file);

        call_dispatcher<float>(args, D, Q);
    }
    else{
        cout <<"USAGE : -type <mnist|sift>"<<endl;
        exit(EXIT_FAILURE);
    }

    return 0;
}