project('gst-opencvserver-plugin', 'c', 'cpp',
    meson_version : '>= 0.63',
    version : '1.0.0',
    default_options : [
        'werror=true',
        'warning_level=2',
        'cpp_std=c++17',
    ],
    license : 'LGPL 2.1+')

gst_opencvserver_plugin_git_version = meson.project_version()

gst_opencvserver_plugin_git_version = gst_opencvserver_plugin_git_version.split('+')[0]
project_version = meson.project_version().split('+')[0]

plugin_c_args = ['-DHAVE_CONFIG_H']

cdata = configuration_data()
cdata.set_quoted('PACKAGE_VERSION', gst_opencvserver_plugin_git_version)
cdata.set_quoted('PACKAGE', meson.project_name())
cdata.set_quoted('GST_LICENSE', 'LGPL')
cdata.set_quoted('GST_API_VERSION', project_version)
cdata.set_quoted('GST_PACKAGE_NAME', 'GStreamer template Plug-ins')
cdata.set_quoted('GST_PACKAGE_ORIGIN', 'https://gstreamer.freedesktop.org')
configure_file(output : 'config.h', configuration : cdata)


# Configure the build environment.
cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')
config_h = configuration_data()

if cc.has_header_symbol('fcntl.h', 'F_ADD_SEALS', prefix : '#define _GNU_SOURCE')
    config_h.set('HAVE_FILE_SEALS', 1)
endif

if cc.has_header_symbol('unistd.h', 'issetugid')
    config_h.set('HAVE_ISSETUGID', 1)
endif

if cc.has_header_symbol('locale.h', 'locale_t', prefix : '#define _GNU_SOURCE')
    config_h.set('HAVE_LOCALE_T', 1)
endif

if cc.has_header_symbol('sys/mman.h', 'memfd_create', prefix : '#define _GNU_SOURCE')
    config_h.set('HAVE_MEMFD_CREATE', 1)
endif

ioctl_posix_test = '''
#include <sys/ioctl.h>
int ioctl (int, int, ...);
'''

if cc.compiles(ioctl_posix_test)
    config_h.set('HAVE_POSIX_IOCTL', 1)
endif

if cc.has_header_symbol('stdlib.h', 'secure_getenv', prefix : '#define _GNU_SOURCE')
    config_h.set('HAVE_SECURE_GETENV', 1)
endif

common_arguments = [
    '-Wmissing-declarations',
    '-Wshadow',
    '-include', meson.current_build_dir() / 'config.h',
]

c_arguments = []
cpp_arguments = []

if cc.get_id() == 'clang'
    if cc.version().version_compare('<9')
        error('clang version is too old, libcamera requires 9.0 or newer')
    endif

    # Turn _FORTIFY_SOURCE by default on. This is needed on clang only as gcc
    # enables it by default. FORTIFY will not work properly with `-O0`, and may
    # result in macro redefinition errors if the user already has a setting for
    # `-D_FORTIFY_SOURCE`. Do not enable FORTIFY in either of those cases.
    if get_option('optimization') != '0'
        fortify = cc.get_define('_FORTIFY_SOURCE')
        if fortify == ''
            message('Adding _FORTIFY_SOURCE')
            common_arguments += [
                '-D_FORTIFY_SOURCE=2',
            ]
        endif
    endif

    # Use libc++ by default if available instead of libstdc++ when compiling
    # with clang.
    if cc.find_library('c++', required : false).found()
        cpp_arguments += [
            '-stdlib=libc++',
        ]
    endif

    cpp_arguments += [
        '-Wextra-semi',
        '-Wthread-safety',
    ]
endif

if cc.get_id() == 'gcc'
    if cc.version().version_compare('<9')
        error('gcc version is too old, libcamera requires 9.0 or newer')
    endif

    # gcc 13 implements the C++23 version of automatic move from local
    # variables in return statements (see
    # https://en.cppreference.com/w/cpp/language/return). As a result, some
    # previously required explicit std::move() in return statements generate
    # warnings. Those moves can't be removed as older compiler versions could
    # use copy constructors instead of move constructors. The easiest fix is to
    # disable the warning. With -Wpessimizing-move enabled, the compiler will
    # still warn of pessimizing moves, only the redundant but not pessimizing
    # moves will be ignored.
    if cc.version().version_compare('>=13')
        cpp_arguments += [
            '-Wno-redundant-move',
        ]
    endif

    # gcc 7.1 introduced processor-specific ABI breakages related to parameter
    # passing on ARM platforms. This generates a large number of messages
    # during compilation. Silence them.
    if host_machine.cpu_family() == 'arm'
        cpp_arguments += [
            '-Wno-psabi',
        ]
    endif
endif

# We use C99 designated initializers for arrays as C++ has no equivalent
# feature. Both gcc and clang support this extension, but recent
# versions of clang generate a warning that needs to be disabled.
if cc.has_argument('-Wno-c99-designator')
    common_arguments += [
        '-Wno-c99-designator',
    ]
endif

c_arguments += common_arguments
cpp_arguments += common_arguments

add_project_arguments(c_arguments, language : 'c')
add_project_arguments(cpp_arguments, language : 'cpp')
add_project_link_arguments(cpp_arguments, language : 'cpp')

subdir('src')
