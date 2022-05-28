#include <pybind11/pybind11.h>

#include <iostream>

#include "aggregation.h"
#include "simple.h"
namespace py = pybind11;

PYBIND11_MODULE(ryuuko, module) {
  module.def("model_mask_parallel", &model_mask_parallel);
  module.def("model_mask", &model_mask);
}