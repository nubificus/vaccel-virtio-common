// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <vaccel.h>

bool vaccel_virtio_pack_validate_path_type(const void *buf, size_t size,
					   uint32_t custom_id);

size_t vaccel_virtio_pack_get_nr_path_args(size_t nr_paths);
int vaccel_virtio_pack_paths(struct vaccel_arg_array *path_args, char **paths,
			     size_t nr_paths);
int vaccel_virtio_unpack_paths(struct vaccel_arg_array *path_args, char **paths,
			       size_t nr_paths);
