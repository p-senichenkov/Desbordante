#pragma once

#include <enum.h>

#include "model/table/column.h"

namespace algos::fastadc {

/**
 * @brief Represents a column operand within a predicate for FastADC.
 *
 * FastADC processes Denial Constraints (DCs) that involve comparisons between
 * pairs of rows within a dataset. A typical DC example, derived from a Functional
 * Dependency such as A -> B, is expressed as: `forall t, s in r, not (t.A = s.A and t.B != s.B)`
 * This denotes that for any pair of rows in the relation, it should not be the case
 * that while the values in column "A" are equal, the values in column "B" are unequal.
 *
 * A predicate in this context (e.g., t.A == s.A) comprises three elements to be fully
 * represented: the column operand from the first tuple ("t.A"), the comparison operator
 * ("="), and the column operand from the second tuple ("s.A"). The `ColumnOperand` class
 * encapsulates the column operand part of a predicate, such as "t.A" or "s.A".
 *
 * The class distinguishes between operands derived from the first tuple (t) and those
 * from the second tuple (s) using an enum `ColumnOperandTuple`, where `ColumnOperandTuple::T`
 * indicates an operand from the first tuple (t), and `ColumnOperandTuple::S` indicates an operand
 * from the second tuple (s).
 */
BETTER_ENUM(ColumnOperandTuple, char, t, s);

// TODO: remove code duplication cause we already have "dc/model/column_operand.h" that is used for
// DC verification.
class ColumnOperand {
private:
    Column const* column_;
    ColumnOperandTuple tuple_;

    static ColumnOperandTuple Invert(ColumnOperandTuple tuple) {
        return tuple == +ColumnOperandTuple::t ? ColumnOperandTuple::s : ColumnOperandTuple::t;
    }

public:
    ColumnOperand(Column const* column, ColumnOperandTuple tuple)
        : column_(column), tuple_(tuple) {}

    bool operator==(ColumnOperand const& rhs) const {
        return column_ == rhs.column_ && tuple_ == rhs.tuple_;
    }

    Column const* GetColumn() const {
        return column_;
    }

    ColumnOperandTuple GetTuple() const {
        return tuple_;
    }

    // here TS means (t, s)
    ColumnOperand GetInvTS() const {
        return ColumnOperand(column_, Invert(tuple_));
    }

    std::string ToString() const {
        return std::string(tuple_._to_string()) + "." + column_->GetName();
    }
};

// NOLINTBEGIN(readability-identifier-naming)
size_t hash_value(ColumnOperand const& k) noexcept;
// NOLINTEND(readability-identifier-naming)

}  // namespace algos::fastadc

template <>
struct std::hash<algos::fastadc::ColumnOperand> {
    size_t operator()(algos::fastadc::ColumnOperand const& k) const noexcept {
        size_t seed = 0;
        boost::hash_combine(seed, k.GetColumn()->GetIndex());
        boost::hash_combine(seed, k.GetTuple()._value);
        return seed;
    }
};
