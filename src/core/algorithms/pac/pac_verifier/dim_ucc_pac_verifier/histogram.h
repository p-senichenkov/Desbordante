#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <limits>
#include <vector>

struct Bucket {
    double min_bound;
    double max_bound;
    std::size_t elem_count;
};

namespace algos::pac_verifier::utils {
class Histogram {
private:
    // Upper bounds of buckets. N - 1 elements (first and last ones are infinite)
    // Upper bound is considered inclusive
    std::vector<double> bucket_bounds_;
    // N buckets
    std::vector<std::size_t> buckets_;

public:
    Histogram(std::size_t bucket_count, double min, double max) : buckets_(bucket_count, 0) {
        assert(bucket_count >= 1);
        bucket_bounds_.reserve(bucket_count);
        double step = (max - min) / bucket_count;
        for (std::size_t i = 1; i < bucket_count; ++i) {
            bucket_bounds_.push_back(step * i);
        }
    }

    void Record(double val, std::size_t count = 1) {
        auto next_bound = std::ranges::upper_bound(bucket_bounds_, val);
        // Buckets are "left-aligned", i. e. bucket_bounds_[0] is upper bound for buckets_[0]
        std::size_t bucket_idx = next_bound == bucket_bounds_.end()
                                         ? buckets_.size() - 1
                                         : std::distance(bucket_bounds_.begin(), next_bound) - 1;
        buckets_[bucket_idx] += count;
    }

    // Perhaps we don't need such a complex representation
    std::vector<Bucket> Buckets() const {
        std::vector<Bucket> result;
        result.reserve(buckets_.size());
        double prev_upper_bound = -std::numeric_limits<double>::infinity();
        for (std::size_t i = 0; i < bucket_bounds_.size(); ++i) {
            auto upper_bound = bucket_bounds_[i];
            result.emplace_back(prev_upper_bound, upper_bound, buckets_[i]);
            prev_upper_bound = upper_bound;
        }
        result.emplace_back(prev_upper_bound, std::numeric_limits<double>::infinity(),
                            buckets_.back());
        return result;
    }
};
}  // namespace algos::pac_verifier::utils
