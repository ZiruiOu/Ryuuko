import numpy as np
from ryuuko import model_mask, model_mask_parallel
from time import time

import numpy as np


def simple_test_ryuuko():
    a = np.array([1+1, 1+2, 1+3], dtype=np.uint64)
    b = np.array([2+1, 2+2, 2+3], dtype=np.uint64)
    c = np.array([3+1, 3+2, 3+3], dtype=np.uint64)

    mask_a = model_mask_parallel(a, 1, 10000)
    mask_b = model_mask_parallel(b, 2, 10000)
    mask_c = model_mask_parallel(c, 3, 10000)

    mask_result = mask_a + mask_b + mask_c

    assert np.all(
        mask_result == 0), "Ryuuko aggregation fail: not all are zero."


def test_ryuuko_overhead(model_size: int) -> float:
    # default : 10 parties
    NUM_RANGE: int = 10
    keys = np.array([i for i in range(1, NUM_RANGE+1)], dtype=np.uint64)
    start_time: float = time()
    mask = model_mask_parallel(keys, 1, model_size)
    end_time: float = time()
    eclapse_time = end_time - start_time
    return mask, eclapse_time


if __name__ == "__main__":
    simple_test_ryuuko()

    result_list = []
    for round in range(50):
        _, eclapse_time = test_ryuuko_overhead(6000000)
        result_list.append(eclapse_time)
    print("eclapse time {:.8f}".format(np.mean(result_list)))
