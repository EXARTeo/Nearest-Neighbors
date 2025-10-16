#pragma once

#include <vector>
#include "../Common/HashTable.hpp"


class LSHHash{
private:

	std::vector<std::vector<unsigned int>> g;

    int LSHSearch();

public:
	LSHHash();
	explicit LSHHash(int dims, int buckets, int L, int k, int w);
	~LSHHash();

};