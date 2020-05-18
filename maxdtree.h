#include "dtree.h"

class MaxDecisionTree : public DecisionTree
{
public:
    MaxDecisionTree(double epsilon = 0);

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
