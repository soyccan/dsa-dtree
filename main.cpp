#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "dtree.h"
#include "tools.h"

int main(int argc, char** argv)
{
    std::cin.tie(NULL);
    std::ios::sync_with_stdio(false);

    std::ifstream infile(argv[1]);
    infile.exceptions(std::ios::failbit);  // throw error if failed opening file

    char* epsilon_end = NULL;
    DecisionTree dtree(std::strtod(argv[2], &epsilon_end));
    if (epsilon_end == argv[2])
        throw std::invalid_argument("Invalid epsilon");

    std::string line, word;
    std::istringstream liness;
    std::vector<std::vector<std::pair<int, double>>> tmp;
    int dimension = 0, num_sample = 0;

    for (num_sample = 0; std::getline(infile, line); num_sample++) {
#ifndef NDEBUG
        // throw internal error to foreground
        liness.exceptions(std::istringstream::failbit);
#endif
        liness.str(line);
        tmp.emplace_back();
        liness >> word;
        tmp.back().emplace_back(std::stoi(word), 0);
        while (liness >> word) {
            auto pos = word.find(':');
            int j = std::stoi(word.substr(0, pos));
            double val = std::stod(word.substr(pos + 1));
            dimension = std::max(dimension, j);
            tmp.back().emplace_back(j, val);
        }
        liness.clear();
    }
    dtree.reset(num_sample, dimension);
    FOR (size_t, i, 0, tmp.size()) {
        dtree.set_result(i + 1, tmp[i][0].first);
        FOR (size_t, j, 1, tmp[i].size()) {
            dtree.set_value(i + 1, tmp[i][j].first, tmp[i][j].second);
        }
    }
    dtree.build();
    dtree.gen_code(std::cout);
    return 0;
}
