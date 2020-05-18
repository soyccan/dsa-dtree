#include "dtree.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <functional>
#include <iomanip>
#include <limits>
#include <memory>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

#include "tools.h"


Node::Node()
    : lch(NULL),
      rch(NULL),
      threshold(std::numeric_limits<double>::max()),
      property(-1)
{
}

Node::~Node()
{
    if (lch)
        delete lch;
    if (rch)
        delete rch;
}


DecisionTree::DecisionTree(double epsilon)
    : __num_sample(0), __dimension(0), __epsilon(epsilon), __root(NULL)
{
}

DecisionTree::~DecisionTree()
{
    if (__root)
        delete __root;
}

void DecisionTree::parseLIBSVM(std::basic_istream<char>& src)
{
    /* in LIBSVM format */

    char buf[0x100];
    char* line;
    char* word;
    std::vector<std::vector<std::pair<int, double>>> tmp;
    int dimension = 0, num_sample = 0;

    for (num_sample = 0; src.good() && src.getline(buf, sizeof buf);
         num_sample++) {
        if (*buf == '\0' || *buf == '\n')
            break;

        line = buf;
        LOG("num_sample: %s line:%s", % num_sample % line);
        word = strsep(&line, " \n");
        LOG("word:%s", % word);

        tmp.emplace_back();
        tmp.back().emplace_back(std::atoi(word), 0);

        while ((word = strsep(&line, " \n"))) {
            if (*word == '\0')
                break;
            LOG("word:%s", % word);

            char* p = strsep(&word, ":");
            assert(word != NULL);

            int j = std::atoi(p);
            double val = std::atof(word);
            dimension = std::max(dimension, j);
            tmp.back().emplace_back(j, val);
        }
    }
    reset(num_sample, dimension);
    FOR (size_t, i, 0, tmp.size()) {
        __res[i] = tmp[i][0].first;
        FOR (size_t, j, 1, tmp[i].size()) {
            __data[__dimension * i + (tmp[i][j].first - 1)] =
                __data_t[__num_sample * (tmp[i][j].first - 1) + i] =
                    tmp[i][j].second;
        }
    }
}

void DecisionTree::reset(int num_sample, int dimension)
{
    __num_sample = num_sample;
    __dimension = dimension;

    if (__data.get())
        __data.release();
    __data = std::make_unique<double[]>(num_sample * dimension);

    if (__data_t.get())
        __data_t.release();
    __data_t = std::make_unique<double[]>(dimension * num_sample);

    if (__res.get())
        __res.release();
    __res = std::make_unique<int[]>(num_sample);

    if (__root)
        delete __root;
    __root = NULL;

    std::srand(std::time(NULL));

    // TODO: 0-init necessary?
    // fill(__data, __data + num_sample * dimension, 0);
    // fill(__data_t, __data_t + num_sample * dimension, 0);
}

void DecisionTree::set_result(int sample, int result)
{
    assert(0 <= sample - 1 && sample - 1 < __num_sample);

    __res[sample - 1] = result;
}

void DecisionTree::set_value(int sample, int property, double value)
{
    assert(0 <= sample - 1 && sample - 1 < __num_sample);
    assert(0 <= property - 1 && property - 1 < __dimension);

    __data[__dimension * (sample - 1) + (property - 1)] =
        __data_t[__num_sample * (property - 1) + (sample - 1)] = value;
}

inline bool DecisionTree::__det_threshold(const int indices[],
                                          int num_indices,
                                          double& threshold,
                                          int& property,
                                          int& l_confusion,
                                          int& r_confusion,
                                          int& l_tendency,
                                          int& r_tendency)
{
    /* determine optimal threshold by minimizing total confusion */

    // total confusion = sum of confusion of left and right
    int min_conf = std::numeric_limits<int>::max();

    // optimal queue, group (threshold, property) pairs of the same total
    // confusion, and randomly choose one to be used
    // thr/prop: threshold and about which property
    // l/rconf: confusion
    // l/rtend: tendency, will be reference of prediction making
    struct __opt_t {
        double thr;
        int prop, lconf, rconf, ltend, rtend;
    };
    std::vector<__opt_t> opt;

    // (val, res)
    auto tmp = std::make_unique<std::pair<double, int>[]>(__num_sample);

    FOR (int, prop, 1, __dimension + 1) {
        int ly = 0, ln = 0, ry = 0, rn = 0;  // left/right yes/no

        FOR (int, i, 0, num_indices) {
            int idx = indices[i];
            double key = __data_t[__num_sample * (prop - 1) + idx - 1];
            int res = __res[idx - 1];

            tmp[i] = std::make_pair(key, res);

            if (res >= 0)
                ry++;
            else
                rn++;
        }
        std::sort(&tmp[0], &tmp[num_indices]);

        FOR (int, i, 0, num_indices - 1) {
            double val;
            int res;
            std::tie(val, res) = tmp[i];

            if (res >= 0)
                ly++, ry--;
            else
                ln++, rn--;

            if (val != tmp[i + 1].first) {
                // handle duplicate keys

                int lconf = std::min(ly, ln);
                int rconf = std::min(ry, rn);
                if (lconf + rconf <= min_conf) {
                    if (lconf + rconf < min_conf) {
                        min_conf = lconf + rconf;
                        opt.clear();
                    }
                    opt.push_back({.thr = (val + tmp[i + 1].first) / 2,
                                   .prop = prop,
                                   .lconf = lconf,
                                   .rconf = rconf,
                                   .ltend = (ly >= ln ? 1 : -1),
                                   .rtend = (ry >= rn ? 1 : -1)});
                }
                LOG(" > prop=%s i=%s lconf=%s rconf=%s ly=%s ln=%s ry=%s rn=%s "
                    "val=%s res=%s",
                    % prop % i % lconf % rconf % ly % ln % ry % rn % val % res);
            }
        }
    }
    if (opt.empty())
        // fail to find threshold
        return false;
    const __opt_t& g = opt[std::rand() % opt.size()];
    threshold = g.thr;
    property = g.prop;
    l_confusion = g.lconf;
    r_confusion = g.rconf;
    l_tendency = g.ltend;
    r_tendency = g.rtend;
    LOG("  threshold=%s prop=%s lconf=%s rconf=%s ltend=%s rtend=%s",
        % threshold % property % l_confusion % r_confusion % l_tendency %
            r_tendency);
    return true;
}

void DecisionTree::__build(Node*& node,
                           int indices[],
                           int num_indices,
                           int depth,
                           int conf,
                           int tend)
{
    if (depth > 1000)
        throw std::runtime_error("depth limit exceed");

    node = new Node();

    if ((double) conf / num_indices <= __epsilon) {
        node->prediction = tend;
        return;
    }

    LOG("\nDepth=%d", % depth);

    int lconf, rconf, ltend, rtend;
    if (!__det_threshold(indices, num_indices, node->threshold, node->property,
                         lconf, rconf, ltend, rtend)) {
        // fail to find a threshold
        node->prediction = tend;
        return;
    }

    int* rpart = std::partition(indices, indices + num_indices, [&](int idx) {
        return __data_t[__num_sample * (node->property - 1) + idx - 1] <=
               node->threshold;
    });
    assert(rpart != indices + num_indices);

    LOGN("  val: ");
    FOR (int, i, 0, num_indices) {
        if (indices + i == rpart)
            LOG("/");
        LOGN(
            "  %s%s",
            % (__res[indices[i] - 1] >= 0 ? '+' : '-') %
                __data_t[__num_sample * (node->property - 1) + indices[i] - 1]);
    }
    LOG("");

    __build(node->lch, indices, rpart - indices, depth + 1, lconf, ltend);
    __build(node->rch, rpart, num_indices - (rpart - indices), depth + 1, rconf,
            rtend);
}

void DecisionTree::build()
{
    int* indices = new int[__num_sample];
    std::iota(&indices[0], &indices[__num_sample], 1);
    __build(__root, indices, __num_sample, 0, std::numeric_limits<int>::max(),
            0);
    delete[] indices;
}

void DecisionTree::__travel_branch(Node* node,
                                   std::basic_ostream<char>& content) const
{
    assert(node);
    if (!node)
        return;

    if (!node->lch && !node->rch) {
        content << "return " << node->prediction << ";";
        return;
    }

    content << "if (attr[" << node->property << "] <= " << node->threshold
            << ") {";
    __travel_branch(node->lch, content);
    content << "} else {";
    __travel_branch(node->rch, content);
    content << "}";
}

void DecisionTree::gen_code(std::basic_ostream<char>& dest) const
{
    dest << std::fixed << std::setprecision(6);
    dest << "int tree_predict(double* attr) {";
    __travel_branch(__root, dest);
    dest << "}";
}

void DecisionTree::gen_code(const std::string& filename) const
{
    std::stringstream content;
    gen_code(content);

    std::ofstream fs(filename);
    fs.exceptions(std::ios::failbit);  // throw error if there is
    fs << content.rdbuf();
}
