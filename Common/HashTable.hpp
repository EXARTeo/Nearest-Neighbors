#include <vector>
#include <cstdint>

template <class T>
struct Entry {
    uint32_t obj_id;              //index to the dataset
    const std::vector<T>* x;      //pointer of the Point
    uint32_t func_id;             //Id based on the caller algorithm
};

//Hash Table (1..to..L)
template <class T>
struct HashTable {
    std::vector<std::vector<Entry<T>>> buckets;     //size = table_size
};
