#include <iostream>
#include <fstream>
#include <vector>
#include <limits>
#include <iomanip>

#include "../Common/Search_dispatcher.hpp"
#include "../Common/Vector_distance.hpp"

#include "../Includes/Call_handler.hpp"
#include "../Includes/LoadData.hpp"
#include "../Includes/Args.hpp"
#include "../Includes/LSH.hpp"


using namespace std;


int main(int argc, char** argv) {

    Args args;
    for (int i = 1 ; i < argc ; i++){
        string s = argv[i];
        if (s == string("-d") && (i + 1 < argc)){
            args.input_file = string(argv[++i]);
        }
        else if (s == string("-q") && (i + 1 < argc)){
            args.query_file = string(argv[++i]);

        }
        else if (s == string("-k") && (i + 1 < argc)){
            args.k = stoi(string(argv[++i]));
        }
        else if (s == string("-L") && (i + 1 < argc)){
            args.L = stoi(string(argv[++i]));
        }
        else if (s == string("-w") && (i + 1 < argc)){
            args.w = stod(string(argv[++i]));
        }
        else if (s == string("-o") && (i + 1 < argc)){
            args.output_file = string(argv[++i]);
        }
        else if (s == string("-N") && (i + 1 < argc)){
            args.N = stoi(string(argv[++i]));
        }
        else if (s == string("-R") && (i + 1 < argc)){
            args.R = stod(string(argv[++i]));
        }
        else if (s == string("-type") && (i + 1 < argc)){
            string temp = string(argv[++i]);
            if(temp == "mnist"){
                args.dataset_type = Type::MNIST;
            }
            else if(temp == "sift"){
                args.dataset_type = Type::SIFT;
            }
            else{
                cout <<"USAGE : -type <mnist|sift>"<<endl;
                exit(EXIT_FAILURE);
            }
        }
        else if (s == string("-range") && (i + 1 < argc)){
            string temp = string(argv[++i]);
            if(temp == "true"){
                args.range = true;
            }
            else if(temp == "false"){
                args.range = false;
            }
            else{
                cout <<"USAGE : -range <true|false>"<<endl;
                exit(EXIT_FAILURE);
            }
        }
        else if (s == string("-seed") && (i + 1 < argc)){
            args.seed = stoi(string(argv[++i]));
        }
        else if (s == string("-lsh") && (i + 1 < argc)){
            args.mode = Mode::LSH;
            continue;
        }
        else{
            cout <<"USAGE : $./search -d <input file> -q <query file> -k <int> -L <int> -w <double> -o <output file> -N <number of nearest> -R <radius> -type <flag> -lsh -range <true|false>"<<endl;
            exit(EXIT_FAILURE);
        }
    }

    //MNIST
    if (args.dataset_type == Type::MNIST) {
        DataImages<uint8_t> D = load_mnist(args.input_file);
        DataImages<uint8_t> Q = load_mnist(args.query_file);

        if (args.R < 0.0) args.R = 2000.0; //default value for MNIST

        call_dispatcher<uint8_t>(args, D, Q);
    }
    //SIFT
    else if(args.dataset_type == Type::SIFT) {
        DataImages<float> D = load_sift(args.input_file);
        DataImages<float> Q = load_sift(args.query_file);

        if (args.R < 0.0) args.R = 2.0; //default value for MNIST

        call_dispatcher<float>(args, D, Q);
    }
    else{
        cout <<"USAGE : -type <mnist|sift>"<<endl;
        exit(EXIT_FAILURE);
    }

    return 0;
}