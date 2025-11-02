#include <iostream>
#include <string>

#include "../Common/Args.hpp"

using namespace std;

//Read through the user input and saves the values
Args load_args(int argc, char** argv){
    Args args;

    int must_give[5] = {0, 0, 0, 0, 0};

    for (int i = 1 ; i < argc ; i++){
        string s = argv[i];
        if (s == string("-d") && (i + 1 < argc)){
            args.input_file = string(argv[++i]);
            must_give[0]++;
        }
        else if (s == string("-q") && (i + 1 < argc)){
            args.query_file = string(argv[++i]);
            must_give[1]++;
        }
        else if (s == string("-o") && (i + 1 < argc)){
            args.output_file = string(argv[++i]);
            must_give[2]++;
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
            must_give[3]++;
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
        }   //LSH only
        else if (s == string("-lsh")){
            args.mode = Mode::LSH;
            must_give[4]++;
        }
        else if (s == string("-k") && (i + 1 < argc)){
            args.k = stoi(string(argv[++i]));
        }
        else if (s == string("-L") && (i + 1 < argc)){
            args.L = stoi(string(argv[++i]));
        }
        else if (s == string("-w") && (i + 1 < argc)){
            args.w = stod(string(argv[++i]));
        }   //Hypercube only
        else if (s == string("-hypercube")){
            args.mode = Mode::Hypercube;
            must_give[4]++;
        }
        else if (s == string("-kproj") && (i + 1 < argc)){
            args.kproj = stoi(string(argv[++i]));
        }
        else if (s == string("-M") && (i + 1 < argc)){
            args.M = stoi(string(argv[++i]));
            args.M_pq = stoi(string(argv[i]));
        }
        else if (s == string("-probes") && (i + 1 < argc)){
            args.probes = stoi(string(argv[++i]));
        }   //IVFFlat / IVFPQ
        else if (s == string("-ivfflat")){
            args.mode = Mode::IVFFlat;
            must_give[4]++;
        }
        else if (s == string("-kclusters") && (i + 1 < argc)){
            args.kclusters = stoi(string(argv[++i]));
        }
        else if (s == string("-nprobe") && (i + 1 < argc)){
            args.nprobe = stoi(string(argv[++i]));
        }   //IVFPQ only
        else if (s == string("-ivfpq")){
            args.mode = Mode::IVFPQ;
            must_give[4]++;
        }
        else if (s == string("-nbits") && (i + 1 < argc)){
            args.nbits = stoi(string(argv[++i]));
        }
        else{
            cout <<"USAGE : $./search -d <input file> -q <query file> -k <int> -L <int> -w <double> -o <output file> -N <number of nearest> -R <radius> -type <flag> -lsh -range <true|false>\n";
            cout <<"USAGE : $./search -d <input file> -q <query file> -kproj <int> -w <double> -M <int> -probes <int> -o <output file> -N <number of nearest> -R <radius> -type <flag> -range <true|false> -hypercube\n";
            cout <<"USAGE : $./search -d <input file> -q <query file> -kclusters <int> -nprobe <int> -o <output file> -N <number of nearest> -R <radius> -type <flag> -range <true|false> -ivfflat -seed <int> \n";
            cout <<"USAGE : $./search -d <input file> -q <query file> -kclusters <int> -nprobe <int> -M <int> -o <output file> -N <number of nearest> -R <radius> -type <flag> -nbits <int> -range <true|false> -ivfpq -seed <int>"<<endl;
            exit(EXIT_FAILURE);
        }
    }

    if (must_give[0] != 1 || must_give[1] != 1 || must_give[2] != 1 || must_give[3] != 1 || must_give[4] != 1){
        cout <<"USAGE : $./search -d <input file> -q <query file> -k <int> -L <int> -w <double> -o <output file> -N <number of nearest> -R <radius> -type <flag> -lsh -range <true|false>\n";
        cout <<"USAGE : $./search -d <input file> -q <query file> -kproj <int> -w <double> -M <int> -probes <int> -o <output file> -N <number of nearest> -R <radius> -type <flag> -range <true|false> -hypercube\n";
        cout <<"USAGE : $./search -d <input file> -q <query file> -kclusters <int> -nprobe <int> -o <output file> -N <number of nearest> -R <radius> -type <flag> -range <true|false> -ivfflat -seed <int> \n";
        cout <<"USAGE : $./search -d <input file> -q <query file> -kclusters <int> -nprobe <int> -M <int> -o <output file> -N <number of nearest> -R <radius> -type <flag> -nbits <int> -range <true|false> -ivfpq -seed <int>"<<endl;
        exit(EXIT_FAILURE);
    }

    if (args.R == -1){
        if (args.dataset_type == Type::MNIST){
            args.R = 2000.0;    //default value for MNIST
        }
        else{
            args.R = 2.0;       //default value for SIFT
        }
    }

    return args;
}