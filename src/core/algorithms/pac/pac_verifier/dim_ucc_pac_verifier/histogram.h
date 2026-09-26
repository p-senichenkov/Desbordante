#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <limits>
#include <vector>

#include "core/util/logger.h"

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

    constexpr static auto kInfinity = std::numeric_limits<double>::infinity();

    std::size_t FindBucket(double val) const {
        auto next_bound = std::ranges::upper_bound(bucket_bounds_, val);
        std::size_t bucket_idx;
        // Buckets are "left-aligned", i. e. bucket_bounds_[0] is upper bound for buckets_[0]
        if (next_bound == bucket_bounds_.begin()) {
            bucket_idx = 0;
        } else if (next_bound == bucket_bounds_.end()) {
            // Last bucket is open
            bucket_idx = buckets_.size() - 1;
        } else {
            bucket_idx = std::distance(bucket_bounds_.begin(), next_bound) - 1;
        }
        LOG_DEBUG("Bucket bounds size: {}, buckets size: {}, bucket idx: {}", bucket_bounds_.size(),
                  buckets_.size(), bucket_idx);
        assert(bucket_idx < buckets_.size());
        return bucket_idx;
    }

public:
    Histogram() = default;

    Histogram(std::size_t bucket_count, double min, double max) : buckets_(bucket_count, 0) {
        assert(bucket_count >= 1);
        bucket_bounds_.reserve(bucket_count);
        double step = (max - min) / bucket_count;
        for (std::size_t i = 1; i < bucket_count; ++i) {
            bucket_bounds_.push_back(step * i);
        }
        assert(bucket_bounds_.size() + 1 == buckets_.size());
    }

    void Record(double val, std::size_t count = 1) {
        // TODO: SIGABRT here
        buckets_[FindBucket(val)] += count;
    }

    // Perhaps we don't need such a complex representation
    std::vector<Bucket> Buckets() const {
        std::vector<Bucket> result;
        result.reserve(buckets_.size());
        double prev_upper_bound = -kInfinity;
        for (std::size_t i = 0; i < bucket_bounds_.size(); ++i) {
            auto upper_bound = bucket_bounds_[i];
            result.emplace_back(prev_upper_bound, upper_bound, buckets_[i]);
            prev_upper_bound = upper_bound;
        }
        result.emplace_back(prev_upper_bound, kInfinity, buckets_.back());
        return result;
    }

    // @return number of elements in bucket, bucket's upper bound
    std::pair<std::size_t, double> GetElemCount(double val) const {
        auto const bucket_idx = FindBucket(val);
        double upper_bound =
                bucket_idx < bucket_bounds_.size() ? bucket_bounds_[bucket_idx] : kInfinity;
        return {buckets_[bucket_idx], upper_bound};
    }
};
}  // namespace algos::pac_verifier::utils
