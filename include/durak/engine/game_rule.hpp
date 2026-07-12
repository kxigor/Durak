#pragma once

#include <cstdint>
#include <string>

#include "engine_fwd.hpp"

namespace durak::engine {

class GameRule {
 public:
  /*======================= Destructors ========================*/
  virtual ~GameRule() = default;

  /*========================= Game API =========================*/
  virtual void apply(DurakEngine& engine) = 0;
  virtual bool is_applicable(DurakEngine& engine) const = 0;
  virtual const std::string& get_name() const noexcept = 0;
};

}  // namespace durak::engine