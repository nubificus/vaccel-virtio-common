project('vaccel-virtio-common', 'c',
  meson_version : '>=1.1',
  default_options : ['c_std=c11'],
  license : 'Apache-2.0',
  license_files : 'LICENSE',
  version : run_command('sh', '-c',
    'git submodule update --init >/dev/null && ' +
    'scripts/common/generate-version.sh .',
    check: true).stdout().strip())

plugin_package = 'vaccel-virtio-common'
plugin_cargs = ['-Wall','-Wextra', '-Wno-unused', '-Werror']

lib_version = meson.project_version().split('-')[0]

libvaccel_dep = dependency('vaccel', version : '>=0.7.0')

subdir('include')
subdir('src')
