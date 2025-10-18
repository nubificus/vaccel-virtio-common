// SPDX-License-Identifier: Apache-2.0

#pragma once

#define common_info(fmt, ...) vaccel_info("[virtio-common] " fmt, ##__VA_ARGS__)
#define common_warn(fmt, ...) vaccel_warn("[virtio-common] " fmt, ##__VA_ARGS__)
#define common_debug(fmt, ...) \
	vaccel_debug("[virtio-common] " fmt, ##__VA_ARGS__)
#define common_error(fmt, ...) \
	vaccel_error("[virtio-common] " fmt, ##__VA_ARGS__)

typedef enum {
	VACCEL_VIRTIO_RESOURCE_REGISTER = 0,
	VACCEL_VIRTIO_RESOURCE_UNREGISTER,
	VACCEL_VIRTIO_GENOP,
	VACCEL_VIRTIO_TORCH_MODEL_LOAD,
	VACCEL_VIRTIO_TORCH_MODEL_RUN,
	VACCEL_VIRTIO_MAX,
} vaccel_virtio_opcode_t;
