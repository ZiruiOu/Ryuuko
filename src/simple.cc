#include "simple.h"

#include <iostream>
#include <limits>
#include <random>

static constexpr uint64_t kDHKeyBase = 191981;
static constexpr uint64_t kDHKeyMod = ((1ULL) << 61) - 1;

static uint64_t fastPower(uint64_t base, uint64_t exp, uint64_t mod) {
  uint64_t result = 1;
  while (exp) {
    if (exp & 1) {
      result = (result * base) % mod;
    }
    base = (base * base) % mod;
    exp >>= 1;
  }
  return result;
}

uint64_t DHKey::getPublicKey() {
  if (!public_key_) {
    public_key_ = fastPower(kDHKeyBase, private_key_, kDHKeyMod);
  }
  return public_key_;
}

void DHKey::exchangeKey(uint64_t public_key) {
  if (!is_exchanged_) {
    exchange_key_ = fastPower(public_key, private_key_, kDHKeyMod);
    is_exchanged_ = true;
  } else {
    std::cerr << "DHKey: duplicated in key exchange." << std::endl;
  }
}

uint64_t DHKey::getExchangeKey() {
  if (is_exchanged_) {
    return exchange_key_;
  } else {
    std::cerr << "DHKey: DHKey not exchanged yet." << std::endl;
    return -1;
  }
}

std::vector<int32_t> model_mask(const std::vector<uint64_t>& exchange_keys,
                                int32_t party_id, int32_t model_size) {
  std::vector<int32_t> mask(model_size);

  for (int i = 0; i < exchange_keys.size(); i++) {
    if (i != party_id - 1) {
      std::default_random_engine engine(exchange_keys[i]);

      std::uniform_int_distribution<int32_t> distribution(
          std::numeric_limits<int32_t>::min(),
          std::numeric_limits<int32_t>::max());

      int sign = (i < party_id - 1) ? 1 : -1;

      for (int j = 0; j < model_size; j++) {
        mask[j] += sign * distribution(engine);
      }
    }
  }

  return mask;
}