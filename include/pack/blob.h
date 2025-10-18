// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <vaccel.h>

bool vaccel_virtio_validate_blob_type(const void *buf, size_t size,
				      uint32_t custom_id);

size_t vaccel_virtio_pack_get_nr_blob_args(size_t nr_blobs);
int vaccel_virtio_pack_blobs(struct vaccel_arg_array *blob_args,
			     struct vaccel_blob **blobs, size_t nr_blobs);
int vaccel_virtio_unpack_blobs(struct vaccel_arg_array *blob_args,
			       struct vaccel_blob **blobs,
			       vaccel_blob_type_t *blob_types, size_t nr_blobs);
