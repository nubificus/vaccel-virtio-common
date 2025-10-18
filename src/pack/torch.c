// SPDX-License-Identifier: Apache-2.0

#include <vaccel-virtio-common/pack/torch.h>

#include <vaccel-virtio-common/core.h>
#include <vaccel-virtio-common/pack/types.h>

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <vaccel.h>

enum { TENSOR_ARGS_NUM = 3 };

bool vaccel_virtio_pack_validate_torch_dtype(const void *buf, size_t size,
					     uint32_t custom_id)
{
	if (!buf || size != sizeof(enum vaccel_torch_data_type))
		return false;
	if (custom_id != VACCEL_VIRTIO_PACK_TORCH_DTYPE)
		return false;

	enum vaccel_torch_data_type value =
		*(const enum vaccel_torch_data_type *)buf;
	return value <= VACCEL_TORCH_FLOAT;
}

size_t vaccel_virtio_pack_get_nr_torch_tensor_args(size_t nr_tensors)
{
	return nr_tensors * TENSOR_ARGS_NUM;
}

int vaccel_virtio_pack_torch_tensors(struct vaccel_arg_array *tensor_args,
				     struct vaccel_torch_tensor **tensors,
				     size_t nr_tensors)
{
	if (!tensor_args || !nr_tensors || !tensors)
		return VACCEL_EINVAL;

	size_t exp_count = nr_tensors * TENSOR_ARGS_NUM;
	if (!tensor_args->owned && exp_count > tensor_args->capacity) {
		common_error(
			"Failed to pack tensors; expected %zu args, got %zu",
			exp_count, tensor_args->capacity);
		return VACCEL_EINVAL;
	}

	int ret;
	for (size_t i = 0; i < nr_tensors; i++) {
		// dims
		ret = vaccel_arg_array_add_int64_array(
			tensor_args, tensors[i]->dims, tensors[i]->nr_dims);
		if (ret) {
			common_error("Failed to pack tensors[%zu].dims arg", i);
			return ret;
		}

		// data
		ret = vaccel_arg_array_add_buffer(tensor_args, tensors[i]->data,
						  tensors[i]->size);
		if (ret) {
			common_error("Failed to pack tensors[%zu].data arg", i);
			return ret;
		}

		// type
		ret = vaccel_arg_array_add_custom(
			tensor_args, VACCEL_VIRTIO_PACK_TORCH_DTYPE,
			&tensors[i]->data_type, sizeof(tensors[i]->data_type),
			vaccel_virtio_pack_validate_torch_dtype);
		if (ret) {
			common_error(
				"Failed to pack tensors[%zu].data_type arg", i);
			return ret;
		}
	}

	return VACCEL_OK;
}

int vaccel_virtio_unpack_torch_tensors(struct vaccel_arg_array *tensor_args,
				       struct vaccel_torch_tensor **tensors,
				       size_t nr_tensors)
{
	if (!tensor_args || !nr_tensors || !tensors)
		return VACCEL_EINVAL;

	// We expect tensor members in the order:
	// 1. dims
	// 2. data
	// 3. type
	size_t rem_count = vaccel_arg_array_remaining_count(tensor_args);
	size_t exp_count = nr_tensors * TENSOR_ARGS_NUM;
	if (exp_count > rem_count) {
		common_error(
			"Failed to unpack tensors; expected %zu args, got %zu",
			exp_count, rem_count);
		return VACCEL_EINVAL;
	}

	for (size_t i = 0; i < nr_tensors; i++)
		tensors[i] = NULL;

	int ret;
	for (size_t i = 0; i < nr_tensors; i++) {
		// dims
		int64_t *dims;
		int64_t nr_dims;
		ret = vaccel_arg_array_get_int64_array(tensor_args, &dims,
						       (size_t *)&nr_dims);
		if (ret) {
			common_error("Failed to unpack tensors[%zu].dims arg",
				     i);
			goto release_tensors;
		}

		// data
		size_t data_idx = vaccel_arg_array_position(tensor_args);
		void *data;
		size_t data_size;
		ret = vaccel_arg_array_get_buffer(tensor_args, &data,
						  &data_size);
		if (ret) {
			common_error("Failed to unpack tensors[%zu].data arg",
				     i);
			goto release_tensors;
		}

		// type
		enum vaccel_torch_data_type *data_type;
		ret = vaccel_arg_array_get_custom(
			tensor_args, VACCEL_VIRTIO_PACK_TORCH_DTYPE,
			(void **)&data_type, NULL,
			vaccel_virtio_pack_validate_torch_dtype);
		if (ret) {
			common_error(
				"Failed to unpack tensors[%zu].data_type arg",
				i);
			goto release_tensors;
		}

		if (tensor_args->args[data_idx].owned) {
			// If data arg is owned use the buffer directly
			ret = vaccel_torch_tensor_new(&tensors[i], nr_dims,
						      dims, *data_type);
			if (ret) {
				common_error("Could not create tensors[%zu]",
					     i);
				goto release_tensors;
			}

			ret = vaccel_torch_tensor_set_data(tensors[i], data,
							   data_size);
			if (ret) {
				common_error(
					"Failed to set data for tensors[%zu]",
					i);
				goto release_tensors;
			}

			// Transfer data ownership to tensor
			tensor_args->args[data_idx].owned = false;
			tensors[i]->owned = true;
		} else {
			// If data arg is not owned allocate a new buffer and
			// copy data
			ret = vaccel_torch_tensor_allocate(&tensors[i], nr_dims,
							   dims, *data_type,
							   data_size);
			if (ret) {
				common_error("Could not create tensors[%zu]",
					     i);
				goto release_tensors;
			}

			memcpy(tensors[i]->data,
			       tensor_args->args[data_idx].buf,
			       tensors[i]->size);
		}
	}

	return VACCEL_OK;

release_tensors:
	for (size_t i = 0; i < nr_tensors; i++)
		if (tensors[i] && vaccel_torch_tensor_delete(tensors[i]))
			common_warn("Could not delete tensors[%zu]", i);

	return ret;
}
