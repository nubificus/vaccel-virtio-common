project('vaccel-virtio-common', 'c',
  meson_version : '>=1.1',
  default_options : ['c_std=c11'],
  license : 'Apache-2.0',
  license_files : 'LICENSE',
  version : run_command('sh', '-c',
    '(git submodule update --init >/dev/null 2>&1 || true) && ' +
    'scripts/common/generate-version.sh',
    env: {'SH_SCRIPTS_DIR': meson.project_source_root() / 'scripts' / 'common'},
    check: true).stdout().strip())

plugin_package = 'vaccel-virtio-common'
plugin_cargs = ['-Wall','-Wextra', '-Wno-unused', '-Werror']

lib_version = meson.project_version().split('-')[0]

libvaccel_dep = dependency('vaccel', version : '>=0.7.0')

subdir('include')
subdir('src')

meson.add_dist_script(
  'scripts/common/dist.sh',
  '-n', meson.project_name(),
  '-v', meson.project_version(),
  '-t', get_option('buildtype'),
  '--pkg-config-path', ':'.join(get_option('pkg_config_path')),
  '--version-only')
