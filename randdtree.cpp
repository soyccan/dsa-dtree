#include "randdtree.h"

#include <algorithm>

#include "tools.h"

RandomDecisionTree::RandomDecisionTree(double epsilon)
    : DecisionTree::DecisionTree(epsilon)
{
}

inline bool RandomDecisionTree::__det_threshold(const int indices[],
                                                int num_indices,
                                                double& threshold,
                                                int& property,
                                                int& l_confusion,
                                                int& r_confusion,
                                                int& l_tendency,
                                                int& r_tendency)
{
    /* determine threshold randomly */

    // (val, res)
    auto tmp = std::make_unique<std::pair<double, int>[]>(__num_sample);

    FOR (int, rand_prop_round, 0, __dimension) {
        // TODO: chances are after __dimension rounds still not reach some prop
        int prop = std::rand() % __dimension + 1;

        FOR (int, i, 0, num_indices) {
            int idx = indices[i];
            double key = __data_t[__num_sample * (prop - 1) + idx - 1];
            int res = __res[idx - 1];

            tmp[i] = std::make_pair(key, res);
        }
        std::sort(&tmp[0], &tmp[num_indices]);

        std::vector<int> thr_idx_cand;
        FOR (int, i, 0, num_indices - 1) {
            if (tmp[i].first != tmp[i + 1].first) {
                thr_idx_cand.push_back(i);
            }
        }
        if (thr_idx_cand.empty()) {
            // this prop can not be used as threshold
            continue;
        }
        int thr_idx = thr_idx_cand[std::rand() % thr_idx_cand.size()];

        int ly = 0, ln = 0, ry = 0, rn = 0;  // left/right yes/no
        FOR (int, i, 0, thr_idx + 1) {
            int res = tmp[i].second;
            if (res >= 0)
                ly++;
            else
                ln++;
        }
        FOR (int, i, thr_idx + 1, num_indices) {
            int res = tmp[i].second;
            if (res >= 0)
                ry++;
            else
                rn++;
        }

        threshold = (tmp[thr_idx].first + tmp[thr_idx + 1].first) / 2;
        property = prop;
        l_confusion = std::min(ly, ln);
        r_confusion = std::min(ry, rn);
        l_tendency = (ly >= ln ? 1 : -1);
        r_tendency = (ry >= rn ? 1 : -1);
        LOG("threshold=%s prop=%s lconf=%s rconf=%s ltend=%s rtend=%s",
            % threshold % property % l_confusion % r_confusion % l_tendency %
                r_tendency);
        return true;
    }
    return false;
}
