// SPDX-License-Identifier: Apache-2.0

#include <vaccel-virtio-common/pack/path.h>

#include <vaccel-virtio-common/core.h>
#include <vaccel-virtio-common/pack/types.h>

#include <stdint.h>
#include <vaccel.h>

bool vaccel_virtio_pack_validate_path_type(const void *buf, size_t size,
					   uint32_t custom_id)
{
	if (!buf || size != sizeof(vaccel_path_type_t))
		return false;
	if (custom_id != VACCEL_VIRTIO_PACK_PATH_TYPE)
		return false;

	vaccel_path_type_t value = *(const vaccel_path_type_t *)buf;
	return value < VACCEL_PATH_MAX;
}

size_t vaccel_virtio_pack_get_nr_path_args(size_t nr_paths)
{
	return nr_paths;
}

int vaccel_virtio_pack_paths(struct vaccel_arg_array *path_args, char **paths,
			     size_t nr_paths)
{
	if (!path_args || !paths || !nr_paths)
		return VACCEL_EINVAL;

	if (!path_args->owned && nr_paths > path_args->capacity) {
		common_error("Failed to pack paths; expected %zu args, got %zu",
			     nr_paths, path_args->capacity);
		return VACCEL_EINVAL;
	}

	for (size_t i = 0; i < nr_paths; i++) {
		int ret = vaccel_arg_array_add_string(path_args, paths[i]);
		if (ret) {
			common_error("Failed to pack path[%zu] arg", i);
			return ret;
		}
	}

	return VACCEL_OK;
}

int vaccel_virtio_unpack_paths(struct vaccel_arg_array *path_args, char **paths,
			       size_t nr_paths)
{
	if (!path_args || !paths || !nr_paths)
		return VACCEL_EINVAL;

	size_t rem_count = vaccel_arg_array_remaining_count(path_args);
	if (nr_paths > rem_count) {
		common_error(
			"Failed to unpack paths; expected %zu args, got %zu",
			nr_paths, rem_count);
		return VACCEL_EINVAL;
	}

	for (size_t i = 0; i < nr_paths; i++) {
		int ret = vaccel_arg_array_get_string(path_args, &paths[i]);
		if (ret) {
			common_error("Failed to unpack path[%zu] arg", i);
			return ret;
		}
	}

	return VACCEL_OK;
}
