// SPDX-License-Identifier: Apache-2.0

#include <vaccel-virtio-common/pack/resource.h>

#include <vaccel-virtio-common/core.h>
#include <vaccel-virtio-common/pack/blob.h>
#include <vaccel-virtio-common/pack/path.h>
#include <vaccel-virtio-common/pack/types.h>

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <vaccel.h>

bool vaccel_virtio_pack_validate_resource_type(const void *buf, size_t size,
					       uint32_t custom_id)
{
	if (!buf || size != sizeof(vaccel_resource_type_t))
		return false;
	if (custom_id != VACCEL_VIRTIO_PACK_RESOURCE_TYPE)
		return false;

	vaccel_resource_type_t value = *(const vaccel_resource_type_t *)buf;
	return value < VACCEL_RESOURCE_MAX;
}

size_t vaccel_virtio_pack_get_nr_resource_args(struct vaccel_resource *res)
{
	if (!res)
		return 0;

	size_t nr_args = 3; // res.type + res.path_type + nr_elems

	if (res->path_type == VACCEL_PATH_REMOTE_FILE)
		nr_args += vaccel_virtio_pack_get_nr_path_args(res->nr_paths);
	else
		nr_args += vaccel_virtio_pack_get_nr_blob_args(res->nr_blobs);

	return nr_args;
}

int vaccel_virtio_pack_resource(struct vaccel_arg_array *res_args,
				struct vaccel_resource *res)
{
	if (!res_args || !res)
		return VACCEL_EINVAL;

	int ret = vaccel_arg_array_add_custom(
		res_args, VACCEL_VIRTIO_PACK_RESOURCE_TYPE, &res->type,
		sizeof(res->type), vaccel_virtio_pack_validate_resource_type);
	if (ret) {
		common_error("Failed to pack resource.type arg");
		return ret;
	}

	ret = vaccel_arg_array_add_custom(
		res_args, VACCEL_VIRTIO_PACK_PATH_TYPE, &res->path_type,
		sizeof(res->path_type), vaccel_virtio_pack_validate_path_type);
	if (ret) {
		common_error("Failed to pack resource.path_type arg");
		return ret;
	}

	if (res->path_type == VACCEL_PATH_REMOTE_FILE) {
		uint32_t *u_nr_paths = malloc(sizeof(uint32_t));
		if (!u_nr_paths)
			return VACCEL_ENOMEM;

		*u_nr_paths = (uint32_t)res->nr_paths;
		ret = vaccel_arg_array_add_uint32(res_args, u_nr_paths);
		if (ret) {
			common_error("Failed to pack nr_paths arg");
			return ret;
		}
		res_args->args[res_args->count - 1].owned = true;

		ret = vaccel_virtio_pack_paths(res_args, res->paths,
					       res->nr_paths);
		if (ret)
			return ret;
	} else {
		uint32_t *u_nr_blobs = malloc(sizeof(uint32_t));
		if (!u_nr_blobs)
			return VACCEL_ENOMEM;

		*u_nr_blobs = (uint32_t)res->nr_blobs;
		ret = vaccel_arg_array_add_uint32(res_args, u_nr_blobs);
		if (ret) {
			common_error("Failed to pack nr_blobs arg");
			return ret;
		}
		res_args->args[res_args->count - 1].owned = true;

		ret = vaccel_virtio_pack_blobs(res_args, res->blobs,
					       res->nr_blobs);
		if (ret)
			return ret;
	}

	return VACCEL_OK;
}

int vaccel_virtio_unpack_resource(struct vaccel_arg_array *res_args,
				  struct vaccel_resource **res)
{
	if (!res_args || !res)
		return VACCEL_EINVAL;

	vaccel_resource_type_t *res_type;
	int ret = vaccel_arg_array_get_custom(
		res_args, VACCEL_VIRTIO_PACK_RESOURCE_TYPE, (void **)&res_type,
		NULL, vaccel_virtio_pack_validate_resource_type);
	if (ret) {
		common_error("Failed to unpack resource.type arg");
		return ret;
	}

	vaccel_path_type_t *path_type;
	ret = vaccel_arg_array_get_custom(
		res_args, VACCEL_VIRTIO_PACK_PATH_TYPE, (void **)&path_type,
		NULL, vaccel_virtio_pack_validate_path_type);
	if (ret) {
		common_error("Failed to unpack resource.path_type arg");
		return ret;
	}

	uint32_t u_nr_elems;
	ret = vaccel_arg_array_get_uint32(res_args, &u_nr_elems);
	if (ret) {
		common_error("Failed to unpack nr_blobs/nr_paths arg");
		return ret;
	}
	size_t nr_elems = (size_t)u_nr_elems;

	if (*path_type == VACCEL_PATH_REMOTE_FILE) {
		char **paths = (char **)malloc(nr_elems * sizeof(const char *));
		if (!paths)
			return VACCEL_ENOMEM;

		ret = vaccel_virtio_unpack_paths(res_args, paths, nr_elems);
		if (ret) {
			free(paths);
			return ret;
		}

		ret = vaccel_resource_multi_new(res, (const char **)paths,
						nr_elems, *res_type);
		free(paths);

		if (ret) {
			common_error("Could not create resource from paths");
			return ret;
		}
	} else {
		struct vaccel_blob **blobs = (struct vaccel_blob **)malloc(
			nr_elems * sizeof(struct vaccel_blob *));
		if (!blobs)
			return VACCEL_ENOMEM;

		vaccel_blob_type_t *blob_types =
			malloc(nr_elems * sizeof(vaccel_blob_type_t));
		if (!blob_types) {
			free(blobs);
			return VACCEL_ENOMEM;
		}

		ret = vaccel_virtio_unpack_blobs(res_args, blobs, blob_types,
						 nr_elems);
		if (ret) {
			free(blob_types);
			free(blobs);
			return ret;
		}

		vaccel_blob_type_t *blob_orig_types =
			malloc(nr_elems * sizeof(vaccel_blob_type_t));
		if (!blob_orig_types)
			return VACCEL_ENOMEM;

		// Set expected blob type, so resource blobs are created
		// correctly
		for (size_t i = 0; i < nr_elems; i++) {
			blob_orig_types[i] = blobs[i]->type;
			blobs[i]->type = blob_types[i];
			blobs[i]->data_owned = true;
		}

		ret = vaccel_resource_from_blobs(
			res, (const struct vaccel_blob **)blobs, nr_elems,
			*res_type);
		if (ret)
			common_error("Could not create resource from blobs");

		for (size_t i = 0; i < nr_elems; i++) {
			if (!blobs[i])
				continue;

			blobs[i]->type = blob_orig_types[i];
			blobs[i]->data_owned = false;

			if (vaccel_blob_delete(blobs[i]))
				common_warn("Could not delete blobs[%zu]", i);
		}

		free(blob_orig_types);
		free(blob_types);
		free(blobs);

		return ret;
	}

	common_debug("Created resource %" PRId64, (*res)->id);
	return VACCEL_OK;
}
