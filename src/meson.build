# SPDX-License-Identifier: CC0-1.0

plugin_c_args = ['-DHAVE_CONFIG_H']

glib_dep = dependency('glib-2.0', required : true )

gst_dep_version = '>=1.14.0'
gstvideo_dep = dependency('gstreamer-video-1.0', version : gst_dep_version,
                          required : true)
gstallocator_dep = dependency('gstreamer-allocators-1.0', version : gst_dep_version,
                              required : true)

opencv_dep = dependency('opencv4', required : true)

if not glib_dep.found() or not gstvideo_dep.found() or not gstallocator_dep.found()
    gst_enabled = false
    subdir_done()
endif

gst_enabled = true

opencvdetector_gst_sources = [
    'gstopencv-utils.cpp',
    'object_detector.cpp',
    'gstopencvserver.cpp',
]

opencvserver_gst_cpp_args = [
    '-DVERSION="@0@"'.format(gst_opencv_detector_git_version),
    '-DPACKAGE="@0@"'.format(meson.project_name()),
    '-DGLIB_VERSION_MIN_REQUIRED=GLIB_VERSION_2_40',
]

# The G_DECLARE_FINAL_TYPE macro creates static inline functions that were
# not marked as possibly unused prior to GLib v2.63.0. This causes clang to
# complain about the ones we are not using. Silence the -Wunused-function
# warning in that case.
if cc.get_id() == 'clang' and glib_dep.version().version_compare('<2.63.0')
    opencvserver_gst_cpp_args += ['-Wno-unused-function']
endif

opencvdetector_gst = shared_library('gstopencvdetector',
    opencvdetector_gst_sources,
    cpp_args : opencvserver_gst_cpp_args,
    dependencies : [gstvideo_dep, gstallocator_dep, opencv_dep],
    install : true,
    install_dir : '@0@/gstreamer-1.0'.format(get_option('libdir')),
)

# Make the plugin visible to GStreamer inside meson devenv.
fs = import('fs')
gst_plugin_path = fs.parent(opencvdetector_gst.full_path())

gst_env = environment()
gst_env.prepend('GST_PLUGIN_PATH', gst_plugin_path)

# Avoid polluting the system registry.
gst_env.set('GST_REGISTRY', gst_plugin_path / 'registry.data')

meson.add_devenv(gst_env)

