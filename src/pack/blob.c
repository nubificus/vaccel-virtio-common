// SPDX-License-Identifier: Apache-2.0

#include <vaccel-virtio-common/pack/blob.h>

#include <vaccel-virtio-common/core.h>
#include <vaccel-virtio-common/pack/types.h>

#include <stddef.h>
#include <stdint.h>
#include <vaccel.h>

enum { BLOB_ARGS_NUM = 3 };

bool vaccel_virtio_validate_blob_type(const void *buf, size_t size,
				      uint32_t custom_id)
{
	if (!buf || size != sizeof(vaccel_blob_type_t))
		return false;
	if (custom_id != VACCEL_VIRTIO_PACK_BLOB_TYPE)
		return false;

	vaccel_blob_type_t value = *(const vaccel_blob_type_t *)buf;
	return value < VACCEL_BLOB_MAX;
}

size_t vaccel_virtio_pack_get_nr_blob_args(size_t nr_blobs)
{
	return nr_blobs * BLOB_ARGS_NUM;
}

int vaccel_virtio_pack_blobs(struct vaccel_arg_array *blob_args,
			     struct vaccel_blob **blobs, size_t nr_blobs)
{
	if (!blob_args || !nr_blobs || !blobs)
		return VACCEL_EINVAL;

	size_t exp_count = nr_blobs * BLOB_ARGS_NUM;
	if (!blob_args->owned && exp_count > blob_args->capacity) {
		common_error("Failed to pack blobs; expected %zu args, got %zu",
			     exp_count, blob_args->capacity);
		return VACCEL_EINVAL;
	}

	int ret;
	for (size_t i = 0; i < nr_blobs; i++) {
		// type
		ret = vaccel_arg_array_add_custom(
			blob_args, VACCEL_VIRTIO_PACK_BLOB_TYPE,
			&blobs[i]->type, sizeof(blobs[i]->type),
			vaccel_virtio_validate_blob_type);
		if (ret) {
			common_error("Failed to pack blobs[%zu].type arg", i);
			return ret;
		}

		// name
		ret = vaccel_arg_array_add_string(blob_args, blobs[i]->name);
		if (ret) {
			common_error("Failed to pack blobs[%zu].name arg", i);
			return ret;
		}

		// data
		ret = vaccel_arg_array_add_buffer(blob_args, blobs[i]->data,
						  blobs[i]->size);
		if (ret) {
			common_error("Failed to pack blobs[%zu].data arg", i);
			return ret;
		}
	}

	return VACCEL_OK;
}

int vaccel_virtio_unpack_blobs(struct vaccel_arg_array *blob_args,
			       struct vaccel_blob **blobs,
			       vaccel_blob_type_t *blob_types, size_t nr_blobs)
{
	if (!blob_args || !blobs || !blob_types || !nr_blobs)
		return VACCEL_EINVAL;

	// We expect required blob members in the order:
	// 1. type
	// 2. name
	// 3. data
	size_t rem_count = vaccel_arg_array_remaining_count(blob_args);
	size_t exp_count = nr_blobs * BLOB_ARGS_NUM;
	if (exp_count > rem_count) {
		common_error(
			"Failed to unpack blobs; expected %zu args, got %zu",
			exp_count, rem_count);
		return VACCEL_EINVAL;
	}

	for (size_t i = 0; i < nr_blobs; i++)
		blobs[i] = NULL;

	int ret;
	for (size_t i = 0; i < nr_blobs; i++) {
		// type
		vaccel_blob_type_t *blob_type;
		ret = vaccel_arg_array_get_custom(
			blob_args, VACCEL_VIRTIO_PACK_BLOB_TYPE,
			(void **)&blob_type, NULL,
			vaccel_virtio_validate_blob_type);
		if (ret) {
			common_error("Failed to unpack blobs[%zu].type arg", i);
			goto free_blobs;
		}
		blob_types[i] = *blob_type;

		// name
		char *blob_name;
		ret = vaccel_arg_array_get_string(blob_args, &blob_name);
		if (ret) {
			common_error("Failed to unpack blobs[%zu].name arg", i);
			goto free_blobs;
		}

		// data
		void *blob_data;
		size_t blob_size;
		ret = vaccel_arg_array_get_buffer(blob_args, &blob_data,
						  &blob_size);
		if (ret) {
			common_error("Failed to unpack blobs[%zu].data arg", i);
			goto free_blobs;
		}

		ret = vaccel_blob_from_buf(&blobs[i], blob_data, blob_size,
					   false, blob_name, NULL, false);
		if (ret) {
			common_error("Could not create blobs[%zus]", i);
			goto free_blobs;
		}
	}

	return VACCEL_OK;

free_blobs:
	for (size_t i = 0; i < nr_blobs; i++)
		if (blobs[i] && vaccel_blob_delete(blobs[i]))
			common_warn("Could not delete blobs[%zu]", i);

	return ret;
}
