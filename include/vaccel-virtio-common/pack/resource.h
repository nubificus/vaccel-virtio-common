// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <vaccel.h>

bool vaccel_virtio_pack_validate_resource_type(const void *buf, size_t size,
					       uint32_t custom_id);

size_t vaccel_virtio_pack_get_nr_resource_args(struct vaccel_resource *res);
int vaccel_virtio_pack_resource(struct vaccel_arg_array *res_args,
				struct vaccel_resource *res);
int vaccel_virtio_unpack_resource(struct vaccel_arg_array *res_args,
				  struct vaccel_resource **res);
