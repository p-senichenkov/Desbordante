#pragma once

#include <chrono>
#include <iostream>
#include <string>

namespace util {
class Benchmarked {
private:
    std::string name_;
    std::chrono::steady_clock::time_point start_;

public:
    explicit Benchmarked(std::string&& name)
        : name_(std::move(name)), start_(std::chrono::steady_clock::now()) {}

    ~Benchmarked() {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start_);
        std::cout << name_ << " took " << elapsed.count() << "ms\n";
    }
};
}  // namespace util
