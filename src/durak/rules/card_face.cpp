#include <cstdint>
#include <include/durak/engine/config.hpp>
#include <include/durak/rules/card_face.hpp>

namespace durak::rules {

CardFace CardFace::from_id(engine::card_id_t id) noexcept {
  const auto kValue = static_cast<std::uint32_t>(id);
  return CardFace{.suit = static_cast<Suit>(kValue & kSuitMask),
                  .rank = static_cast<Rank>(
                      ((kValue >> kRankShift) & kRankMask) + kMinRank)};
}

engine::card_id_t CardFace::to_id() const noexcept {
  const auto kSuitBits = static_cast<std::uint32_t>(suit);
  const auto kRankBits = static_cast<std::uint32_t>(rank) - kMinRank;
  return static_cast<engine::card_id_t>(kSuitBits | (kRankBits << kRankShift));
}

}  // namespace durak::rules
