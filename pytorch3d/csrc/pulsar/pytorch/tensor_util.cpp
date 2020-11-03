// Copyright (c) Facebook, Inc. and its affiliates. All rights reserved.
#include <ATen/cuda/CUDAContext.h>
#include <cuda_runtime_api.h>
#include <torch/extension.h>

#include "./tensor_util.h"

namespace pulsar {
namespace pytorch {

torch::Tensor sphere_ids_from_result_info_nograd(
    const torch::Tensor& forw_info) {
  torch::Tensor result = torch::zeros(
      {forw_info.size(0),
       forw_info.size(1),
       forw_info.size(2),
       (forw_info.size(3) - 3) / 2},
      torch::TensorOptions().device(forw_info.device()).dtype(torch::kInt32));
  // Get the relevant slice, contiguous.
  torch::Tensor tmp =
      forw_info
          .slice(
              /*dim=*/3, /*start=*/3, /*end=*/forw_info.size(3), /*step=*/2)
          .contiguous();
  if (forw_info.device().type() == c10::DeviceType::CUDA) {
    cudaMemcpyAsync(
        result.data_ptr(),
        tmp.data_ptr(),
        sizeof(uint32_t) * tmp.size(0) * tmp.size(1) * tmp.size(2) *
            tmp.size(3),
        cudaMemcpyDeviceToDevice,
        at::cuda::getCurrentCUDAStream());
  } else {
    memcpy(
        result.data_ptr(),
        tmp.data_ptr(),
        sizeof(uint32_t) * tmp.size(0) * tmp.size(1) * tmp.size(2) *
            tmp.size(3));
  }
  // `tmp` is freed after this, the memory might get reallocated. However,
  // only kernels in the same stream should ever be able to write to this
  // memory, which are executed only after the memcpy is complete. That's
  // why we can just continue.
  return result;
}

} // namespace pytorch
} // namespace pulsar
