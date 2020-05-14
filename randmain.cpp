#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "randdtree.h"
#include "tools.h"

int main(int argc, char** argv)
{
    std::cin.tie(NULL);
    std::ios::sync_with_stdio(false);

    if (argc <= 2) {
        std::cerr << "Usage: " << argv[0] << " [train data file] [epsilon]\n";
        std::exit(2);
    }

    std::ifstream infile(argv[1]);
    infile.exceptions(std::ios::failbit);  // throw error if failed opening file

    char* epsilon_end = NULL;
    double epsilon = std::strtod(argv[2], &epsilon_end);
    if (epsilon_end == argv[2])
        throw std::invalid_argument("Invalid epsilon");

    RandomDecisionTree dtree(epsilon);
    dtree.parseLIBSVM(infile);
    dtree.build();
    dtree.gen_code(std::cout);

    return 0;
}
