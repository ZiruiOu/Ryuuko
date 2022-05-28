#include "aggregation.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <random>

#include "omp.h"
#include "threadpool.h"

namespace py = pybind11;

static constexpr int32_t kNumThreads = 5;

static void localMaskGenerator(const std::vector<uint64_t>& seeds,
                               std::vector<int32_t>& local_mask,
                               int start_party, int num_jobs, int my_party,
                               int model_size) {
  for (int i = 0; i < num_jobs; i++) {
    int current_party = start_party + i;
    if (current_party == my_party - 1) {
      continue;
    }
    std::mt19937 engine(seeds[current_party]);
    std::uniform_int_distribution<int32_t> distribution(
        std::numeric_limits<int32_t>::min(),
        std::numeric_limits<int32_t>::max());
    int sign = (current_party < my_party - 1) ? 1 : -1;
    for (int j = 0; j < model_size; j++) {
      int32_t temp = sign * distribution(engine);
      local_mask[j] += temp;
    }
  }
}

py::array_t<int32_t> model_mask_parallel(
    const py::array_t<uint64_t>& exchange_keys, int party_id, int model_size) {
  auto mask = py::array_t<int32_t>(model_size);
  auto key_array = exchange_keys.unchecked<1>();

  py::buffer_info mask_info = mask.request();
  int* mask_ptr = static_cast<int*>(mask_info.ptr);
  std::memset(mask_ptr, 0, mask.nbytes());

  int num_parties = exchange_keys.size();

  std::vector<uint64_t> seeds;
  std::vector<int> local_masks[kNumThreads];
  for (int i = 0; i < num_parties; i++) {
    seeds.push_back(key_array(i));
  }

  for (int i = 0; i < kNumThreads; i++) {
    local_masks[i].assign(model_size, 0);
  }

  int start_index = 0;
  int num_jobs = 0;
  int jobs_per_worker = num_parties / kNumThreads;
  int remains = num_parties % kNumThreads;

  {
    ThreadPoolPtr pool = std::make_unique<ThreadPool>((size_t)kNumThreads);

    for (int i = 0; i < kNumThreads; i++) {
      num_jobs = jobs_per_worker;

      if (i < remains) {
        num_jobs += 1;
      }

      pool->submit(std::bind(localMaskGenerator, std::cref(seeds),
                             std::ref(local_masks[i]), start_index, num_jobs,
                             party_id, model_size));
      start_index += num_jobs;
    }
    pool->start();
    pool->stop();
  }

  for (int i = 0; i < model_size; i++) {
    for (int j = 0; j < kNumThreads; j++) {
      mask_ptr[i] += local_masks[j][i];
    }
  }

  return mask;
}

py::array_t<int32_t> model_mask(const py::array_t<uint64_t>& exchange_keys,
                                int party_id, int model_size) {
  auto mask = py::array_t<int32_t>(model_size);
  py::buffer_info mask_info = mask.request();
  int32_t* mask_ptr = static_cast<int32_t*>(mask_info.ptr);

  int32_t num_parties = exchange_keys.size();
  auto key_array = exchange_keys.unchecked<1>();
  std::memset(mask_ptr, 0, mask.nbytes());

  for (int i = 0; i < num_parties; i++) {
    if (i == party_id - 1) {
      continue;
    }
    uint64_t seed = key_array(i);
    std::default_random_engine engine(seed);
    std::uniform_int_distribution<int32_t> distribution(
        std::numeric_limits<int32_t>::min(),
        std::numeric_limits<int32_t>::max());
    int sign = (i < party_id - 1) ? 1 : -1;
    for (int j = 0; j < model_size; j++) {
      int temp = sign * distribution(engine);
      { mask_ptr[j] += temp; }
    }
  }
  return mask;
}