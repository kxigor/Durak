#include <gtest/gtest.h>

#include <cstdint>
#include <include/durak/engine/modulo.hpp>
#include <stdexcept>

namespace {

using Ring = durak::modulo::ModuloRing<std::int8_t>;

// --- Construction ---

TEST(ModuloTest, DefaultIsZeroRing) {
  Ring r;
  EXPECT_EQ(r.get_mod(), 0);
  EXPECT_EQ(r.as<int>(), 0);
}

TEST(ModuloTest, ModOnlyConstruct) {
  Ring r{5};
  EXPECT_EQ(r.get_mod(), 5);
  EXPECT_EQ(r.as<int>(), 0);
}

TEST(ModuloTest, ValueInRange) {
  Ring r{5, 3};
  EXPECT_EQ(r.as<int>(), 3);
}

TEST(ModuloTest, ValueWrapsWhenTooLarge) {
  Ring r{5, 7};
  EXPECT_EQ(r.as<int>(), 2);
}

TEST(ModuloTest, ValueWrapsNegative) {
  Ring r{5, -1};
  EXPECT_EQ(r.as<int>(), 4);
}

TEST(ModuloTest, ValueWrapsLargeNegative) {
  Ring r{5, -7};
  EXPECT_EQ(r.as<int>(), 3);
}

// --- Copy / assignment ---

TEST(ModuloTest, CopyPreservesState) {
  Ring a{7, 4};
  Ring b{a};
  EXPECT_EQ(b.get_mod(), 7);
  EXPECT_EQ(b.as<int>(), 4);
}

TEST(ModuloTest, AssignPreservesState) {
  Ring a{7, 4};
  Ring b;
  b = a;
  EXPECT_EQ(b.get_mod(), 7);
  EXPECT_EQ(b.as<int>(), 4);
}

// --- Setters ---

TEST(ModuloTest, SetNumAndMod) {
  Ring r;
  r.set_mod(6);
  r.set_num(2);
  EXPECT_EQ(r.get_mod(), 6);
  EXPECT_EQ(r.get_num(), 2);
}

// --- Factory ---

TEST(ModuloTest, CreateKeepsModulus) {
  Ring base{5, 1};
  Ring made = base.create(9);  // 9 % 5 == 4
  EXPECT_EQ(made.get_mod(), 5);
  EXPECT_EQ(made.as<int>(), 4);
}

// --- Conversion ---

TEST(ModuloTest, ImplicitToNum) {
  Ring r{5, 3};
  std::int8_t v = r;
  EXPECT_EQ(static_cast<int>(v), 3);
}

// --- Addition (ring) ---

TEST(ModuloTest, PlusEqualsWraps) {
  Ring r{5, 3};
  r += Ring{5, 4};  // 7 % 5 == 2
  EXPECT_EQ(r.as<int>(), 2);
}

TEST(ModuloTest, PlusReturnsNewRing) {
  Ring a{5, 3};
  Ring b = a + Ring{5, 4};
  EXPECT_EQ(b.as<int>(), 2);
  EXPECT_EQ(a.as<int>(), 3);  // original untouched
}

// --- Subtraction (ring) ---

TEST(ModuloTest, MinusEqualsWrapsNegative) {
  Ring r{5, 1};
  r -= Ring{5, 3};  // -2 -> 3
  EXPECT_EQ(r.as<int>(), 3);
}

TEST(ModuloTest, MinusReturnsNewRing) {
  Ring a{5, 1};
  Ring b = a - Ring{5, 3};
  EXPECT_EQ(b.as<int>(), 3);
  EXPECT_EQ(a.as<int>(), 1);
}

// --- Arithmetic with raw number ---

TEST(ModuloTest, PlusEqualsNumber) {
  Ring r{5, 3};
  r += std::int8_t{4};
  EXPECT_EQ(r.as<int>(), 2);
}

TEST(ModuloTest, MinusEqualsNumber) {
  Ring r{5, 1};
  r -= std::int8_t{3};
  EXPECT_EQ(r.as<int>(), 3);
}

TEST(ModuloTest, PlusNumber) {
  Ring r = Ring{5, 3} + std::int8_t{4};
  EXPECT_EQ(r.as<int>(), 2);
}

TEST(ModuloTest, MinusNumber) {
  Ring r = Ring{5, 1} - std::int8_t{3};
  EXPECT_EQ(r.as<int>(), 3);
}

TEST(ModuloTest, NegativeOffsetWalksBackwards) {
  Ring r{5, 0};
  r += std::int8_t{-1};  // 0 - 1 -> 4
  EXPECT_EQ(r.as<int>(), 4);
}

// --- gcd ---

TEST(ModuloTest, GcdCoprime) { EXPECT_EQ(Ring::gcd(3, 5), 1); }

TEST(ModuloTest, GcdCommonDivisor) { EXPECT_EQ(Ring::gcd(2, 4), 2); }

TEST(ModuloTest, GcdNegativeIsAbsolute) { EXPECT_EQ(Ring::gcd(-1, 5), 1); }

// --- covers_full_ring ---

TEST(ModuloTest, CoversWhenCoprime) {
  Ring r{5};
  EXPECT_TRUE(r.covers_full_ring(2));
}

TEST(ModuloTest, DoesNotCoverWhenSharedDivisor) {
  Ring r{4};
  EXPECT_FALSE(r.covers_full_ring(2));
}

TEST(ModuloTest, ZeroModulusNeverCovers) {
  Ring r;  // modulus 0
  EXPECT_FALSE(r.covers_full_ring(1));
}

// --- Zero modulus guard ---

TEST(ModuloTest, ZeroModulusValueConstructThrows) {
  EXPECT_THROW((Ring{0, 1}), std::logic_error);
}

TEST(ModuloTest, ArithmeticOnZeroModulusThrows) {
  Ring r;  // modulus 0
  EXPECT_THROW((r += Ring{0, 0}), std::logic_error);
}

}  // namespace