#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include "tests/accuracy/output.h"
#include "tests/accuracy/runner.h"
#include "tests/accuracy/ucc_pac_accuracy_tests.h"

namespace po = boost::program_options;

class AccuracyCLI {
private:
    std::string output_;
    std::vector<std::size_t> n_;

    po::options_description desc_{"Options"};

public:
    AccuracyCLI() {
        desc_.add_options()("output,o", po::value(&output_), "Output filename")(
                "numeric,n", po::value(&n_), "Numeric charactericstics");
    }

    void Parse(int argc, char** argv) {
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc_), vm);

        auto output = vm["output"];
        if (output.empty()) {
            throw std::runtime_error("required option: --output");
        }
        output_ = output.as<std::string>();

        auto numeric = vm["numeric"];
        if (numeric.empty()) {
            throw std::runtime_error("required option: --numeric");
        }
        n_ = numeric.as<std::vector<std::size_t>>();
    }

    std::string const& Output() const {
        return output_;
    }

    std::vector<std::size_t> const& N() const {
        return n_;
    }
};

int main(int argc, char** argv) {
    AccuracyCLI cli;
    cli.Parse(argc, argv);

    accuracy::AccuracyTestRunner runner;
    accuracy::RegisterUCCPACAccuracyTests(runner);

    auto results = runner.RunAll(cli.N());
    accuracy::Save(results, cli.Output());
}
