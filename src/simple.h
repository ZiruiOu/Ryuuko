#ifndef RYUUKO_SRC_SIMPLE_H
#define RYUUKO_SRC_SIMPLE_H

#include <cstddef>
#include <cstdint>
#include <vector>

class DHKey {
 public:
  DHKey(uint64_t private_key)
      : is_exchanged_(false), private_key_(private_key) {}
  ~DHKey() = default;

  void exchangeKey(uint64_t public_key);
  uint64_t getPublicKey();
  uint64_t getExchangeKey();

 private:
  bool is_exchanged_;
  uint64_t public_key_{0};
  uint64_t private_key_;
  uint64_t exchange_key_{0};
};

std::vector<int32_t> model_mask_vector(
    const std::vector<uint64_t>& exchange_keys, int32_t party_id,
    int32_t model_size);

#endif  // ! RYUUKO_SRC_SIMPLE_H