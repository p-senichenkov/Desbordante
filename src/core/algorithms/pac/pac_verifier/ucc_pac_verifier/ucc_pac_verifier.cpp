#include "core/algorithms/pac/pac_verifier/ucc_pac_verifier/ucc_pac_verifier.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "core/algorithms/pac/model/tuple_type.h"
#include "core/algorithms/pac/pac_verifier/options.h"
#include "core/algorithms/pac/pac_verifier/pac_man.h"
#include "core/algorithms/pac/pac_verifier/ucc_pac_verifier/ucc_pac_highlight.h"
#include "core/algorithms/pac/pac_verifier/util/make_tuples.h"
#include "core/algorithms/pac/pac_verifier/util/pac_verifier_utils.h"
#include "core/algorithms/pac/pac_verifier/util/tuple_pair.h"
#include "core/algorithms/pac/pac_verifier/util/validation_mode.h"
#include "core/algorithms/pac/ucc_pac.h"
#include "core/config/custom_metric/custom_vector_metric/option.h"
#include "core/config/descriptions.h"
#include "core/config/indices/option.h"
#include "core/config/names.h"
#include "core/config/option_using.h"
#include "core/config/tabular_data/input_table/option.h"
#include "core/util/logger.h"

namespace algos::pac_verifier {
double UCCPACVerifier::EmpiricalProbabilitiesCalculator::GetNumPairs(double delta) const {
    // delta = (2 * pairs + total_tuples) / total_tuples^2
    // => pairs = (delta * total_tuples^2 - total_tuples) / 2 =
    //          = total_tuples * (delta * total_tuples - 1) / 2
    double num_pairs = static_cast<double>(tuple_count_ * (delta * tuple_count_ - 1)) / 2;
    if (num_pairs < 0) {
        num_pairs = 0;
    }
    return num_pairs;
}

double UCCPACVerifier::EmpiricalProbabilitiesCalculator::GetDelta(std::size_t num_pairs) const {
    // See "key ideas"
    double delta = static_cast<double>(2 * num_pairs + tuple_count_) / std::pow(tuple_count_, 2);
    assert(delta >= 0 && delta <= 1);
    return delta;
}

void UCCPACVerifier::EmpiricalProbabilitiesCalculator::RefineDelta(PairsIt& iter) const {
    if (iter == sorted_pairs_.begin()) {
        return;
    }
    auto initial_eps = iter->dist;
    while (iter != sorted_pairs_.begin() && initial_eps - iter->dist < kDistThreshold) {
        std::advance(iter, -1);
    }
    // This iterator is inclusive, not "past-the-", so `advance` is not needed here
}

void UCCPACVerifier::PreparePairs() {
    sorted_pairs_ = std::make_shared<std::vector<TuplePair>>();
    auto const total_tuples = tuples_->size();
    sorted_pairs_->reserve((total_tuples * (total_tuples - 1)) / 2);
    for (std::size_t i = 0; i < total_tuples; ++i) {
        for (std::size_t j = i + 1; j < total_tuples; ++j) {
            auto const& first = (*tuples_)[i];
            auto const& second = (*tuples_)[j];
            sorted_pairs_->emplace_back(i, j, (*metric_)(first, second));
        }
    }
    std::ranges::sort(*sorted_pairs_, {}, [](TuplePair const& p) { return p.dist; });
}

void UCCPACVerifier::LoadDataInternal() {
    typed_relation_ = model::ColumnLayoutTypedRelationData::CreateFrom(*input_table_, true);
    input_table_->Reset();
    if (typed_relation_->GetColumnData().empty()) {
        throw std::runtime_error("Got an empty dataset: PAC validation is meaningless.");
    }

    std::vector<model::Type const*> types(column_indices_.size());
    auto const& col_data = typed_relation_->GetColumnData();
    std::ranges::transform(column_indices_, types.begin(),
                           [&col_data](std::size_t const idx) { return &col_data[idx].GetType(); });

    std::vector<std::string> column_names;
    column_names.reserve(column_indices_.size());
    std::ranges::transform(
            column_indices_, std::back_inserter(column_names),
            [&col_data](std::size_t idx) { return col_data[idx].GetColumn()->GetName(); });
    metric_ = metric_opt_->SetTypes(types, column_names);

    tuple_type_ = std::make_shared<pac::model::TupleType>(std::move(types));

    tuples_ = pac::util::MakeTuples(typed_relation_->GetColumnData(), column_indices_);
}

void UCCPACVerifier::ExecuteInternal() {
    LOG_DEBUG("Common PAC verifier options: delta: [0, {}], eps: [{}, {}], delta steps: {}",
              max_delta_, min_epsilon_, max_epsilon_, delta_steps_);

    std::ostringstream oss;
    oss << '{';
    for (auto it = column_indices_.begin(); it != column_indices_.end(); ++it) {
        if (it != column_indices_.begin()) {
            oss << ", ";
        }
        oss << *it;
    }
    oss << '}';
    LOG_INFO("Verifying UCC PAC on columns {}", oss.str());

    EpsilonDelta eps_delta;
    auto [mode, eps_override, delta_override] =
            SelectPACValidationMode(mode_, min_epsilon_, max_epsilon_, 0, max_delta_);
    switch (mode) {
        case PACValidationMode::kFindPac:
            eps_delta = FindPAC();
            break;
        case PACValidationMode::kFindEpsilon:
            if (delta_override) {
                delta_ = *delta_override;
            }
            eps_delta = FindEpsilon();
            break;
        case PACValidationMode::kFindDelta:
            if (eps_override) {
                epsilon_ = *eps_override;
            }
            eps_delta = FindDelta();
            break;
    }

    Vertical columns = typed_relation_->GetSchema()->GetVertical(
            util::IndicesToBitset(column_indices_, typed_relation_->GetNumColumns()));
    pac_ = model::UCCPAC{std::move(columns), eps_delta.epsilon, eps_delta.delta};

    LOG_INFO("Result: {}", pac_->ToLongString());
}

EpsilonDelta UCCPACVerifier::FindPAC() {
    PreparePairs();

    EmpiricalProbabilitiesCalculator empirical_probabilities_calculator{
            max_delta_, delta_steps_, *sorted_pairs_, tuples_->size()};
    PACMan pac_man{
            min_epsilon_,
            max_epsilon_,
            0,
            max_delta_,
            diagonal_threshold_,
            [empirical_probabilities_calculator](double epsilon) {
                return empirical_probabilities_calculator.GetEpsilonDeltaForEpsilonImpl(epsilon);
            }};

    auto emp_probabilities = empirical_probabilities_calculator.CalculateEmpiricalProbabilities();
    return pac_man.FindEpsilonDelta(std::move(emp_probabilities));
}

EpsilonDelta UCCPACVerifier::FindEpsilon() {
    std::size_t const total_pairs = std::pow(tuples_->size(), 2);

    std::size_t total_needed_pairs = std::ceil(static_cast<double>(total_pairs) * delta_);
    // needed_pairs is total_needed_pairs \cap lower triangle
    // All next computations are performed only on the lower triangle
    // delta = (needed_pairs * 2 + tuples.size()) / total_pairs = total_needed_pairs / total_pairs
    std::size_t needed_pairs = std::ceil((total_needed_pairs - tuples_->size()) / 2);

    std::vector<double> distances;
    distances.reserve(tuples_->size() * (tuples_->size() - 1) / 2);
    for (std::size_t i = 0; i < tuples_->size() - 1; ++i) {
        for (std::size_t j = i + 1; j < tuples_->size(); ++j) {
            distances.push_back((*metric_)((*tuples_)[i], (*tuples_)[j]));
        }
    }

    std::ranges::nth_element(distances, std::next(distances.begin(), needed_pairs));
    auto epsilon = distances[needed_pairs];
    // TODO: This statement needs more thought...
    // Actual refinement (i. e. finding max delta with the same epsilon) would be too expensive and
    // ruin all optimizations
    // But since each pair has a counterpart, we can refine a little for no cost
    if ((total_needed_pairs - tuples_->size()) % 2 == 0) {
        return {epsilon, static_cast<double>(total_needed_pairs + 1) / total_pairs};
    }
    return {epsilon, delta_};
}

EpsilonDelta UCCPACVerifier::FindDelta() {
    if (!std::isfinite(epsilon_)) {
        double max_dist = 0;
        for (std::size_t i = 0; i < tuples_->size() - 1; ++i) {
            for (std::size_t j = i + 1; j < tuples_->size(); ++j) {
                max_dist = std::max(max_dist, (*metric_)((*tuples_)[i], (*tuples_)[j]));
            }
        }
        return {max_dist, 1};
    }

    std::size_t lower_count = tuples_->size();
    for (std::size_t i = 0; i < tuples_->size() - 1; ++i) {
        for (std::size_t j = i + 1; j < tuples_->size(); ++j) {
            auto dist = (*metric_)((*tuples_)[i], (*tuples_)[j]);
            if (dist <= epsilon_) {
                lower_count += 2;
            }
        }
    }
    return {epsilon_, static_cast<double>(lower_count) / std::pow(tuples_->size(), 2)};
}

UCCPACVerifier::UCCPACVerifier() {
    DESBORDANTE_OPTION_USING;
    using namespace config;
    using enum PACValidationMode;

    // Load data options
    RegisterOption(kTableOpt(&input_table_).SetConditionalOpts({{nullptr, {kColumnIndices}}}));
    RegisterOption(IndicesOption{kColumnIndices, kDColumnIndices, nullptr}(
            &column_indices_, [this]() { return input_table_->GetNumberOfColumns(); }));
    RegisterOption(VectorMetricOption(&metric_opt_));

    MakeOptionsAvailable({kTableOpt.GetName(), kCustomMetric});

    auto get_min_epsilon = [this]() { return min_epsilon_; };
    auto get_max_epsilon = [this]() { return max_epsilon_; };
    auto get_max_delta = [this]() { return max_delta_; };

    // Execute options
    RegisterOption(PACValidationModeOption(
            &mode_,
            {{kFindPac, {kMinEpsilon}}, {kFindEpsilon, {kDelta}}, {kFindDelta, {kEpsilon}}}));

    RegisterOption(kMinEpsilonOption(&min_epsilon_).SetConditionalOpts({{nullptr, {kMaxEpsilon}}}));
    RegisterOption(MaxEpsilonOption(get_min_epsilon)(&max_epsilon_)
                           .SetConditionalOpts({{nullptr, {kMaxDelta}}}));
    RegisterOption(MaxDeltaOption(get_min_epsilon, get_max_epsilon)(&max_delta_)
                           .SetConditionalOpts({{nullptr, {kDeltaSteps}}}));
    RegisterOption(DeltaStepsOption([]() { return 0; }, get_max_delta)(&delta_steps_));
    RegisterOption(kDiagonalThresholdOption(&diagonal_threshold_));

    RegisterOption(kEpsilonOption(&epsilon_));
    RegisterOption(kDeltaOption(&delta_));
}

UCCPACHighlight UCCPACVerifier::GetHighlights(double eps_1, double eps_2) const {
    if (!pac_) {
        throw std::runtime_error("Execute must be called before calling GetHighlights");
    }

    if (eps_2 < 0) {
        eps_2 = pac_->GetEpsilon();
    }
    if (eps_2 <= eps_1) {
        return UCCPACHighlight{};
    }
    LOG_DEBUG("Calculating highlights from {} to {}...", eps_1, eps_2);

    auto begin = std::ranges::upper_bound(*sorted_pairs_, eps_1, {},
                                          [](TuplePair const& pair) { return pair.dist; });
    auto end = std::ranges::upper_bound(begin, sorted_pairs_->end(), eps_2, {},
                                        [](TuplePair const& pair) { return pair.dist; });

    LOG_DEBUG("Highlighted pairs [{}, {})", std::distance(sorted_pairs_->begin(), begin),
              std::distance(sorted_pairs_->begin(), end));
    return UCCPACHighlight{tuples_, tuple_type_, sorted_pairs_, std::move(begin), std::move(end)};
}
}  // namespace algos::pac_verifier
