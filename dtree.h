/* convinient functions */
#ifndef _DSA_DTREE_H_
#define _DSA_DTREE_H_ 1

#include <memory>

struct Node {
    // lch: values that <= threshold; rch: otherwise
    Node *lch, *rch;

    // following are valid only for non-leaves
    double threshold;  // value of the property <= threshold will go on to left
                       // child, otherwise right
    int property;      // which property is used in comparison with threshold
                       // (1-indexed)

    // following are value only for leaves (lch == rch == NULL)
    int prediction;  // predicted result

    Node();
    ~Node();
};


class DecisionTree
{
public:
    DecisionTree(double epsilon = 0);
    ~DecisionTree();

    void reset(int num_sample, int dimension);
    void set_result(int sample, int result);
    void set_value(int sample, int property, double value);

    void parseLIBSVM(std::basic_istream<char>& src);
    void build();
    void gen_code(std::basic_ostream<char>& dest) const;
    void gen_code(const std::string& filename) const;

protected:
    virtual inline bool __det_threshold(const int indices[],
                                        int num_indices,
                                        double& threshold,
                                        int& property,
                                        int& l_confusion,
                                        int& r_confusion,
                                        int& l_tendency,
                                        int& r_tendency);

    void __build(Node*& node,
                 int indices[],
                 int num_indices,
                 int depth,
                 int conf,
                 int tend);

    void __travel_branch(Node* node, std::basic_ostream<char>& content) const;


    int __num_sample, __dimension;
    double __epsilon;
    Node* __root;
    std::unique_ptr<double[]> __data;    // [i][j]: j-th property of i-th sample
    std::unique_ptr<double[]> __data_t;  // transposed
    std::unique_ptr<int[]> __res;        // [i]: result of i-th sample
};

#endif
