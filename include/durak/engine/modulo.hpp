#pragma once

#include <numeric>
#include <stdexcept>

namespace durak::modulo {

template <typename NumT>
class ModuloRing {
  using num_t = NumT;

 public:
  /*======================= Constructors =======================*/
  ModuloRing() = default;

  explicit ModuloRing(num_t mod) noexcept : mod_{std::move(mod)} {}

  ModuloRing(num_t mod, num_t num)
      : mod_{std::move(mod)}, num_{std::move(num)} {
    wrap();
  }

  ModuloRing(const ModuloRing& /*unused*/) = default;
  ModuloRing(ModuloRing&& /*unused*/) = default;

  /*======================= Assignments ========================*/
  ModuloRing& operator=(const ModuloRing& /*unused*/) = default;
  ModuloRing& operator=(ModuloRing&& /*unused*/) = default;

  /*======================= Destructors ========================*/
  ~ModuloRing() = default;

  /*=================== Arithmetic operators ===================*/
  ModuloRing& operator+=(const ModuloRing& rhs) {
    num_ += rhs.num_;
    wrap();
    return *this;
  }

  ModuloRing& operator-=(const ModuloRing& rhs) {
    num_ -= rhs.num_;
    wrap();
    return *this;
  }

  ModuloRing operator+(const ModuloRing& rhs) const {
    return ModuloRing{*this} += rhs;
  }

  ModuloRing operator-(const ModuloRing& rhs) const {
    return ModuloRing{*this} -= rhs;
  }

  ModuloRing& operator+=(num_t rhs) { return *this += create(rhs); }

  ModuloRing& operator-=(num_t rhs) { return *this -= create(rhs); }

  ModuloRing operator+(num_t rhs) const { return *this + create(rhs); }

  ModuloRing operator-(num_t rhs) const { return *this - create(rhs); }

  /*========================== Fabric ==========================*/
  [[nodiscard]] ModuloRing create(num_t num) const {
    return ModuloRing{mod_, num};
  }

  /*======================= Ring queries =======================*/
  // A step generates the whole ring iff it is coprime with the modulus.
  // Used at start-up to reject offsets that would skip players.
  static num_t gcd(num_t a, num_t b) noexcept {
    return static_cast<num_t>(std::gcd(a, b));
  }

  [[nodiscard]] bool covers_full_ring(num_t step) const noexcept {
    return mod_ != NumT{0} && gcd(step, mod_) == NumT{1};
  }

  /*========================= Setters ==========================*/
  void set_num(num_t num) noexcept { num_ = std::move(num); }

  void set_mod(num_t mod) noexcept { mod_ = std::move(mod); }

  /*========================= Getters ==========================*/
  [[nodiscard]] const num_t& get_num() const noexcept { return num_; }

  [[nodiscard]] const num_t& get_mod() const noexcept { return mod_; }

  // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
  operator num_t() const noexcept { return num_; }

  template <typename AsT>
  [[nodiscard]] AsT as() const noexcept {
    return static_cast<AsT>(num_);
  }

 private:
  /*========================= Helpers ==========================*/
  // Normalizes num_ into [0, mod_) and guards against a zero modulus,
  // which would make the modulo operation undefined.
  void wrap() {
    if (mod_ == NumT{0}) {
      throw std::logic_error("ModuloRing: modulus must be non-zero");
    }
    num_ = static_cast<num_t>(num_ % mod_);
    if (num_ < NumT{0}) {
      num_ = static_cast<num_t>(num_ + mod_);
    }
  }

  /*======================= Data fields ========================*/
  num_t mod_{};
  num_t num_{};
};

}  // namespace durak::modulo