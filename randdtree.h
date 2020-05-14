#include "dtree.h"

class RandomDecisionTree : public DecisionTree
{
public:
    RandomDecisionTree(double epsilon = 0);

protected:
    inline bool __det_threshold(const int indices[],
                                int num_indices,
                                double& threshold,
                                int& property,
                                int& l_confusion,
                                int& r_confusion,
                                int& l_tendency,
                                int& r_tendency) override;
};
