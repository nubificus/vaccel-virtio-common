common_sources = []

subdir('pack')

libvaccel_virtio_common = static_library('libvaccel-virtio-common',
  common_sources,
  c_args : plugin_cargs,
  include_directories : common_includes,
  dependencies : libvaccel_dep,
  install : false)

libvaccel_virtio_common_dep = declare_dependency(
  sources : common_headers,
  include_directories : common_includes,
  dependencies : libvaccel_dep,
  link_with: libvaccel_virtio_common)
