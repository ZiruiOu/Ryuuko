#ifndef RYUUKO_SRC_AGGREGATION_H
#define RYUUKO_SRC_AGGREGATION_H

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include <vector>

pybind11::array_t<int32_t> model_mask(
    const pybind11::array_t<uint64_t>& exchange_keys, int32_t party_id,
    int32_t model_size);

pybind11::array_t<int32_t> model_mask_parallel(
    const pybind11::array_t<uint64_t>& exchange_keys, int32_t party_id,
    int32_t model_size);

#endif  // ! RYUUKO_SRC_AGGREGATION_H