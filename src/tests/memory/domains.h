#pragma once

#include <memory>
#include <string>
#include <vector>

#include "core/algorithms/pac/model/default_domains/ball.h"
#include "core/algorithms/pac/model/idomain.h"

namespace memory {
static std::shared_ptr<pac::model::IDomain> const kIowaLowArityBall =
        std::make_shared<pac::model::Ball>(
                std::vector<std::string>{"3", "Good Goods", "40", "John Doe"}, 15);
}  // namespace memory
