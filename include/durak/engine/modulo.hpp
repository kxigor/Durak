#pragma once

#include <type_traits>

namespace durak::modulo {

template <typename NumT>
class ModuloRing {
  using num_t = NumT;

 public:
  /*======================= Constructors =======================*/
  ModuloRing() = default;

  ModuloRing(num_t mod) : mod_{std::move(mod)} {}

  ModuloRing(num_t mod, num_t num)
      : mod_{std::move(mod)}, num_{std::move(num)} {
    if (num_ < NumT{0}) {
      num_ %= mod_;
      num_ += mod_;
    }
    num_ %= mod_;
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
    num_ %= mod_;
    return *this;
  }

  ModuloRing& operator-=(const ModuloRing& rhs) {
    if (num_ < rhs_num) {
      num_ += mod_;
    }
    num_ -= rhs.num_;
    num_ %= mod_;
    return *this;
  }

  ModuloRing operator+(const ModuloRing& rhs) const {
    return ModuloRing{*this} += rhs;
  }

  ModuloRing operator-(const ModuloRing& rhs) const {
    return ModuloRing{*this} -= rhs;
  }

  /*========================== Fabric ==========================*/
  ModuloRing create(num_t num) const { return ModuloRing{mod_, num}; }

  /*========================= Setters ==========================*/
  void set_num(num_t num) { num_ = std::move(num); }

  void set_mod(num_t mod) { mod_ = std::move(mod); }

  /*========================= Getters ==========================*/
  const std::pair<num_t, num_t> get_pair() const { return {num_, mod_}; }

  const num_t& get_num() const { return num_; }

  const num_t& get_mod() const { return mod_; }

  operator num_t() const { return num_; }

  template <typename AsT>
  AsT as() const {
    return static_cast<AsT>(num_);
  }

 private:
  /*======================= Data fields ========================*/
  num_t mod_{};
  num_t num_{};
};

};  // namespace durak::modulo