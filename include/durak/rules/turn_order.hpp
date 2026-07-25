#pragma once

#include <include/durak/engine/config.hpp>
#include <include/durak/engine/engine_fwd.hpp>

namespace durak::rules {

[[nodiscard]] engine::modulo_base_t first_active_from(
    const engine::GameRuleView& view, engine::modulo_base_t start);

}  // namespace durak::rules
