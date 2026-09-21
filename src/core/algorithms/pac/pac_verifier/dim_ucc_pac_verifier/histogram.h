#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <format>
#include <fstream>
#include <iterator>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include "core/util/logger.h"

namespace algos::pac_verifier::utils {
namespace detail {
struct Bucket {
    double upper_bound;
    std::size_t elem_count;
};

class BucketStorage {
private:
    constexpr static auto kInfinity = std::numeric_limits<double>::infinity();

    // Upper bounds of buckets. N - 1 elements (first and last ones are infinite)
    // Bucket is (,]
    std::vector<double> bucket_bounds_;
    // N buckets
    std::vector<std::size_t> buckets_;

    static std::vector<detail::Bucket> Strip(std::vector<detail::Bucket> const& orig_buckets) {
        std::vector<detail::Bucket> new_buckets;
        for (auto const& bucket : orig_buckets) {
            if (bucket.elem_count > 0) {
                new_buckets.push_back(bucket);
            } else {
                if (new_buckets.empty() || new_buckets.back().elem_count > 0) {
                    new_buckets.push_back(bucket);
                } else {
                    new_buckets.back().upper_bound = bucket.upper_bound;
                }
            }
        }
        return new_buckets;
    }

public:
    BucketStorage() : bucket_bounds_(), buckets_{0} {}

    explicit BucketStorage(std::vector<double>&& bucket_bounds)
        : bucket_bounds_(std::move(bucket_bounds)),
          buckets_(std::vector<std::size_t>(bucket_bounds_.size() + 1, 0)) {}

    BucketStorage(std::vector<double>&& bucket_bounds, std::vector<std::size_t>&& buckets)
        : bucket_bounds_(std::move(bucket_bounds)), buckets_(std::move(buckets)) {
        assert(bucket_bounds_.size() + 1 == buckets_.size());
        assert(std::ranges::is_sorted(bucket_bounds_));
    }

    std::vector<std::size_t>& ElemCounts() {
        return buckets_;
    }

    std::size_t FindBucket(double val) const {
        // XXX: remove extra asserts
        assert(std::ranges::is_sorted(bucket_bounds_));
        auto next_bound = std::ranges::upper_bound(bucket_bounds_, val);
        std::size_t bucket_idx;
        // Buckets are "left-aligned", i. e. bucket_bounds_[0] is upper bound for buckets_[0]
        if (next_bound == bucket_bounds_.begin()) {
            bucket_idx = 0;
        } else if (next_bound == bucket_bounds_.end()) {
            // Last bucket is open
            bucket_idx = buckets_.size() - 1;
        } else {
            assert(std::distance(bucket_bounds_.begin(), next_bound) > 0);
            bucket_idx = std::distance(bucket_bounds_.begin(), next_bound) - 1;
        }
        // TODO: Still fails here...
        LOG_DEBUG("Bucket idx: {}, bucket bounds size: {}, buckets size: {}", bucket_idx,
                  bucket_bounds_.size(), buckets_.size());
        assert(bucket_idx < buckets_.size());
        return bucket_idx;
    }

    // @return number of elements in bucket, bucket's upper bound
    std::pair<std::size_t, double> Get(double val) const {
        auto const bucket_idx = FindBucket(val);
        double upper_bound =
                bucket_idx < bucket_bounds_.size() ? bucket_bounds_[bucket_idx] : kInfinity;
        return {buckets_[bucket_idx], upper_bound};
    }

    void Add(double val, std::size_t elem_count) {
        buckets_[FindBucket(val)] += elem_count;
    }

    std::vector<Bucket> GetBuckets() const {
        std::vector<detail::Bucket> result;
        result.reserve(buckets_.size());
        for (std::size_t i = 0; i < bucket_bounds_.size(); ++i) {
            result.push_back(Bucket{bucket_bounds_[i], buckets_[i]});
        }
        result.push_back(Bucket{kInfinity, buckets_.back()});
        return result;
    }

    std::vector<Bucket> StrippedBuckets() const {
        return Strip(GetBuckets());
    }
};
}  // namespace detail

class Histogram {
private:
    detail::BucketStorage storage_;

public:
    Histogram() = default;

    Histogram(std::size_t bucket_count, double min, double max) {
        assert(bucket_count >= 1);
        std::vector<double> bucket_bounds(bucket_count);
        double step = (max - min) / bucket_count;
        for (std::size_t i = 1; i < bucket_count; ++i) {
            bucket_bounds.push_back(step * i);
        }

        storage_ = detail::BucketStorage{std::move(bucket_bounds)};
    }

    void Record(double val, std::size_t count = 1) {
        storage_.Add(val, count);
    }

    // @return number of elements in bucket, bucket's upper bound
    std::pair<std::size_t, double> Get(double val) const {
        return storage_.Get(val);
    }

    std::string ToString() const {
        std::ostringstream out;
        for (auto const& bucket : storage_.StrippedBuckets()) {
            out << std::format("{} {}]\n", bucket.elem_count, bucket.upper_bound);
        }
        return out.str();
    }

    void Dump(std::string_view const& filename) const {
        std::ofstream ofs{filename.data()};
        for (auto const& bucket : storage_.StrippedBuckets()) {
            ofs << std::format("{} {}\n", bucket.elem_count, bucket.upper_bound);
        }
    }

    std::vector<detail::Bucket> Buckets() const {
        return storage_.GetBuckets();
    }
};

// ECDF is basically the same thing as histogram, but meaning is different
class Ecdf {
private:
    detail::BucketStorage storage_;

public:
    Ecdf(std::vector<double> bucket_bounds, std::vector<std::size_t>&& elem_counts)
        : storage_(std::move(bucket_bounds), std::move(elem_counts)) {}

    // @return number of elements in bucket, bucket's upper bound
    std::pair<std::size_t, double> Get(double val) const {
        return storage_.Get(val);
    }
};
}  // namespace algos::pac_verifier::utils
