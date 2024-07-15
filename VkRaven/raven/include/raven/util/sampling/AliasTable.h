#pragma once
#include <cassert>
#include <cstdint>
#include <numeric>
#include <vector>

namespace raven {
    class AliasTable {
    public:
        explicit AliasTable(std::vector<float> weights) : m_bins(weights.size()) {
            // Normalize _weights_ to compute alias table PDF
            const float sum = std::accumulate(weights.begin(), weights.end(), 0.);
            assert(sum > 0);
            for (size_t i = 0; i < weights.size(); ++i)
                m_bins[i].p = weights[i] / sum;

            // Create alias table work lists
            struct Outcome {
                float pHat;
                size_t index;
            };
            std::vector<Outcome> under, over;
            for (size_t i = 0; i < m_bins.size(); ++i) {
                // Add outcome _i_ to an alias table work list
                const float pHat = m_bins[i].p * m_bins.size();
                if (pHat < 1)
                    under.push_back(Outcome{pHat, i});
                else
                    over.push_back(Outcome{pHat, i});
            }

            // Process under and over work item together
            while (!under.empty() && !over.empty()) {
                // Remove items _un_ and _ov_ from the alias table work lists
                const Outcome un = under.back(), ov = over.back();
                under.pop_back();
                over.pop_back();

                // Initialize probability and alias for _un_
                m_bins[un.index].q = un.pHat;
                m_bins[un.index].alias = ov.index;

                // Push excess probability on to work list
                const float pExcess = un.pHat + ov.pHat - 1;
                if (pExcess < 1)
                    under.push_back(Outcome{pExcess, ov.index});
                else
                    over.push_back(Outcome{pExcess, ov.index});
            }

            // Handle remaining alias table work items
            while (!over.empty()) {
                const Outcome ov = over.back();
                over.pop_back();
                m_bins[ov.index].q = 1;
                m_bins[ov.index].alias = -1;
            }
            while (!under.empty()) {
                const Outcome un = under.back();
                under.pop_back();
                m_bins[un.index].q = 1;
                m_bins[un.index].alias = -1;
            }
        }

        int32_t sample(float xi, float *pdf) const {
            // Compute alias table _offset_ and remapped random sample _up_
            const int offset = std::min<int>(xi * m_bins.size(), m_bins.size() - 1);
            const float up = std::min<float>(xi * m_bins.size() - offset, 0x1.fffffep-1);

            if (up < m_bins[offset].q) {
                // Return sample for alias table at _offset_
                assert(m_bins[offset].p > 0);
                if (pdf)
                    *pdf = m_bins[offset].p;
                return offset;
            } else {
                // Return sample for alias table at _alias[offset]_
                const int alias = m_bins[offset].alias;
                assert(alias >= 0);
                assert(m_bins[alias].p > 0);
                if (pdf)
                    *pdf = m_bins[alias].p;
                return alias;
            }
        }

    // private:
        struct Bin {
            float q, p;
            int32_t alias;
        };

        std::vector<Bin> m_bins;
    };
} // namespace raven