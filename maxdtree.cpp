#include "maxdtree.h"

#include <algorithm>

#include "tools.h"

MaxDecisionTree::MaxDecisionTree(double epsilon)
    : DecisionTree::DecisionTree(epsilon)
{
}

inline bool MaxDecisionTree::__det_threshold(const int indices[],
                                             int num_indices,
                                             double& threshold,
                                             int& property,
                                             int& l_confusion,
                                             int& r_confusion,
                                             int& l_tendency,
                                             int& r_tendency)
{
    /* determine optimal threshold by maximizing total confusion */

    // total confusion = sum of confusion of left and right
    int max_conf = std::numeric_limits<int>::min();

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
                if (lconf + rconf >= max_conf) {
                    if (lconf + rconf > max_conf) {
                        max_conf = lconf + rconf;
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
