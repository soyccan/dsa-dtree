#include "dtree.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <limits>
#include <memory>
#include <numeric>
#include <sstream>
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


DecisionTree::DecisionTree() : __num_sample(0), __dimension(0), __epsilon(0) {}

DecisionTree::DecisionTree(double epsilon)
    : __num_sample(0), __dimension(0), __epsilon(epsilon)
{
}

DecisionTree::~DecisionTree()
{
    if (__root)
        delete __root;
}

const Node* DecisionTree::root() const
{
    return __root;
}

void DecisionTree::reset(int num_sample, int dimension)
{
    // LOG("reset n=%d d=%d", % num_sample % dimension);

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
    // __root = new Node();

    // __tree = std::make_unique<double[]>(num_sample * num_sample);

    // TODO: 0-init necessary?
    // fill(__data, __data + num_sample * dimension, 0);
    // fill(__data_t, __data_t + num_sample * dimension, 0);
    // fill(__tree, __tree + num_sample * num_sample, 0);
}

void DecisionTree::set_result(int sample, int result)
{
    // LOG("set_result sample=%d result=%d", % sample % result);

    assert(0 <= sample - 1 && sample - 1 < __num_sample);

    __res[sample - 1] = result;
}

void DecisionTree::set_value(int sample, int property, double value)
{
    // LOG("set_value sample=%d property=%d value=%lf",
    //     % sample % property % value);

    assert(0 <= sample - 1 && sample - 1 < __num_sample);
    assert(0 <= property - 1 && property - 1 < __dimension);

    __data[__dimension * (sample - 1) + (property - 1)] =
        __data_t[__num_sample * (property - 1) + (sample - 1)] = value;
}

inline void DecisionTree::__det_threshold(const int indices[],
                                          int num_indices,
                                          double& threshold,
                                          int& property,
                                          int& l_confusion,
                                          int& r_confusion,
                                          int& l_tendency,
                                          int& r_tendency)
{
    /* determine optimal threshold by minimizing total confusion */

    // optimal threshold and to what property it's related
    double opt_thr = NAN;
    int opt_prop = -1;

    // confusion
    int opt_lconf = -1, opt_rconf = -1;
    int min_conf = std::numeric_limits<int>::max();

    // tendency, will be reference of prediction making
    int opt_ltend = -1, opt_rtend = -1;

    // (val, res, idx)
    auto tmp = std::make_unique<std::tuple<double, int, int>[]>(__num_sample);

    FOR (int, prop, 1, __dimension + 1) {
        int ly = 0, ln = 0, ry = 0, rn = 0;  // left/right yes/no

        FOR (int, i, 0, num_indices) {
            int idx = indices[i];
            tmp[i] =
                std::make_tuple(__data_t[__num_sample * (prop - 1) + idx - 1],
                                __res[idx - 1], idx);
            if (__res[idx - 1] >= 0)
                ry++;
            else
                rn++;
        }
        std::sort(&tmp[0], &tmp[num_indices]);

        FOR (int, i, 0, num_indices - 1) {
            double val;
            int res;
            int idx;
            std::tie(val, res, idx) = tmp[i];

            if (res >= 0)
                ly++, ry--;
            else
                ln++, rn--;

            int lconf = std::min(ly, ln);
            int rconf = std::min(ry, rn);
            if (lconf + rconf < min_conf) {
                min_conf = lconf + rconf;
                opt_thr = val;
                opt_prop = prop;
                opt_lconf = lconf;
                opt_rconf = rconf;
                opt_ltend = ly > ln ? 1 : -1;
                opt_rtend = ry > rn ? 1 : -1;
            }
            // LOG("prop=%s i=%s conf=%s val=%s res=%s idx=%s",
            //     % prop % i % conf % val % res % idx);
        }
    }
    threshold = opt_thr;
    property = opt_prop;
    l_confusion = opt_lconf;
    r_confusion = opt_rconf;
    l_tendency = opt_ltend;
    r_tendency = opt_rtend;
    LOG("threshold=%s prop=%s lconf=%s rconf=%s ltend=%s rtend=%s",
        % threshold % property % l_confusion % r_confusion % l_tendency %
            r_tendency);
}

void DecisionTree::__build(Node*& node, int indices[], int num_indices)
{
    node = new Node();

    int lconf, rconf, ltend, rtend;
    __det_threshold(indices, num_indices, node->threshold, node->property,
                    lconf, rconf, ltend, rtend);

    int* rpart = std::partition(indices, indices + num_indices, [&](int idx) {
        return __data_t[__num_sample * (node->property - 1) + idx - 1] <=
               node->threshold;
    });

    LOGN("val: ");
    FOR (int, i, 0, num_indices) {
        if (indices + i == rpart)
            LOG("/");
        LOGN(
            "%s-%s ",
            % indices[i] %
                __data_t[__num_sample * (node->property - 1) + indices[i] - 1]);
    }
    LOG("");

    bool l_is_leaf = (double) lconf / num_indices <= __epsilon;
    bool r_is_leaf = (double) rconf / num_indices <= __epsilon;

    if (!l_is_leaf) {
        __build(node->lch, indices, rpart - indices);
    } else {
        node->lch = new Node();
        node->lch->prediction = ltend;
    }

    if (!r_is_leaf) {
        __build(node->rch, rpart, num_indices - (rpart - indices));
    } else {
        node->rch = new Node();
        node->rch->prediction = rtend;
    }
}

void DecisionTree::build()
{
    int* indices = new int[__num_sample];
    std::iota(&indices[0], &indices[__num_sample], 1);
    __build(__root, indices, __num_sample);
    delete[] indices;
}

void DecisionTree::__travel_branch(Node* node,
                                   std::basic_ostream<char>& content) const
{
    if (!node->lch && !node->rch) {
        content << "return " << node->prediction << ";";
        return;
    }

    content << "if (attr[" << node->property << "-1] <= " << node->threshold
            << ") {";
    __travel_branch(node->lch, content);
    content << "} else {";
    __travel_branch(node->rch, content);
    content << "}";
}

void DecisionTree::gen_code(std::basic_ostream<char>& dest) const
{
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
