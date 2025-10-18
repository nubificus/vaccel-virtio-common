// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <vaccel.h>

bool vaccel_virtio_pack_validate_torch_dtype(const void *buf, size_t size,
					     uint32_t custom_id);

size_t vaccel_virtio_pack_get_nr_torch_tensor_args(size_t nr_tensors);
int vaccel_virtio_pack_torch_tensors(struct vaccel_arg_array *tensor_args,
				     struct vaccel_torch_tensor **tensors,
				     size_t nr_tensors);
int vaccel_virtio_unpack_torch_tensors(struct vaccel_arg_array *tensor_args,
				       struct vaccel_torch_tensor **tensors,
				       size_t nr_tensors);
