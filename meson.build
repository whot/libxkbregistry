project(
    'libxkbregistry',
    'c',
    version: '0.0.1',
    default_options: [
        'c_std=c99',
        'warning_level=2',
        'b_lundef=true',
    ],
    meson_version : '>= 0.41.0',
)
pkgconfig = import('pkgconfig')
cc = meson.get_compiler('c')
dep_libxml = dependency('libxml-2.0')

# Compiler flags.
foreach cflag: [
    '-fvisibility=hidden',
    '-fno-strict-aliasing',
    '-fsanitize-undefined-trap-on-error',
    '-Wextra',
    '-Wno-unused-parameter',
    '-Wno-missing-field-initializers',
    '-Wpointer-arith',
    '-Wmissing-declarations',
    '-Wformat=2',
    '-Wstrict-prototypes',
    '-Wmissing-prototypes',
    '-Wnested-externs',
    '-Wbad-function-cast',
    '-Wshadow',
    '-Wlogical-op',
    '-Wdate-time',
    '-Wwrite-strings',
    '-Wno-documentation-deprecated-sync',
]
    if cc.has_argument(cflag)
        add_project_arguments(cflag, language: 'c')
    endif
endforeach

# The XKB config root.
XKBCONFIGROOT = get_option('xkb-config-root')
if XKBCONFIGROOT == ''
    xkeyboard_config_dep = dependency('xkeyboard-config', required: false)
    if xkeyboard_config_dep.found()
        XKBCONFIGROOT = xkeyboard_config_dep.get_pkgconfig_variable('xkb_base')
    else
        XKBCONFIGROOT = join_paths(get_option('prefix'), get_option('datadir'), 'X11', 'xkb')
  endif
endif

# config.h.
configh_data = configuration_data()
# Like AC_USE_SYSTEM_EXTENSIONS, what #define to use to get extensions
# beyond the base POSIX function set.
if host_machine.system() == 'sunos'
  system_extensions = '__EXTENSIONS__'
else
  system_extensions = '_GNU_SOURCE'
endif
configh_data.set(system_extensions, 1)
system_ext_define = '#define ' + system_extensions
configh_data.set_quoted('DFLT_XKB_CONFIG_ROOT', XKBCONFIGROOT)
if cc.has_header_symbol('unistd.h', 'eaccess', prefix: system_ext_define)
    configh_data.set('HAVE_EACCESS', 1)
endif
if cc.has_header_symbol('unistd.h', 'euidaccess', prefix: system_ext_define)
    configh_data.set('HAVE_EUIDACCESS', 1)
endif

configure_file(output: 'config.h', configuration: configh_data)
add_project_arguments('-include', 'config.h', language: 'c')

# Supports -Wl,--version-script?
have_version_script = cc.links(
    'int main(){}',
    args: '-Wl,--version-script=' + join_paths(meson.source_root(), 'src', 'xkbregistry.map'),
    name: '-Wl,--version-script',
)


libxkbregistry_sources = [
    'src/xkbregistry.c',
    'src/xkbregistry.h',
    'src/xkbregistry-private.h',
    'src/util-list.c',
    'src/util-list.h',
]
libxkbregistry_link_args = []
if have_version_script
    libxkbregistry_link_args += '-Wl,--version-script=' + join_paths(meson.source_root(), 'src', 'xkbregistry.map')
endif
deps_libxkbregistry = [dep_libxml]
libxkbregistry = library(
    'xkbregistry',
    libxkbregistry_sources,
    link_args: libxkbregistry_link_args,
    dependencies: deps_libxkbregistry,
    link_depends: join_paths(meson.source_root(), 'src', 'xkbregistry.map'),
    version: '0.0.0',
    install: true,
    include_directories: include_directories('src'),
)
dep_libxkbregistry = declare_dependency(link_with: libxkbregistry,
					dependencies: deps_libxkbregistry)
install_headers(
    'src/xkbregistry.h',
    subdir: 'xkbregistry',
)
pkgconfig.generate(
    name: 'xkbregistry',
    filebase: 'xkbregistry',
    libraries: libxkbregistry,
    version: meson.project_version(),
    description: 'XKB registry API to list available RMLVO components',
)

# Tests
test_env = environment()
test_env.set('top_srcdir', meson.source_root())
test_env.set('top_builddir', meson.build_root())
test_env.set('MALLOC_PERTURB_', '15')
test_env.set('MallocPreScribble', '1')
test_env.set('MallocScribble', '1')

libxkbregistry_test_internal = static_library(
    'xkbregistry-test-internal',
    'test/test.h',
    libxkbregistry_sources,
    include_directories: include_directories('src'),
    dependencies: deps_libxkbregistry,
)
test_dep = declare_dependency(
    include_directories: include_directories('src'),
    link_with: libxkbregistry_test_internal,
)
test(
    'test-registry',
    executable('test-registry', 'test/test-registry.c', dependencies: test_dep),
    env: test_env,
)


executable('xkbregistry-list-all',
	    'examples/list-all.c',
	    dependencies: [dep_libxkbregistry],
	    include_directories: include_directories('src'),
	    install: false)
# Documentation.
if get_option('enable-docs')
    doxygen = find_program('doxygen', required: false)
    if not doxygen.found()
        error('''Documentation requires doxygen which was not found.
You can disable the documentation with -Denable-docs=false.''')
    endif
    doxygen_wrapper = find_program('scripts/doxygen-wrapper')

    doxygen_input = [
        'README.md',
        'doc/doxygen-extra.css',
        'src/xkbregistry.h',
    ]
    doxygen_data = configuration_data()
    doxygen_data.set('PACKAGE_NAME', meson.project_name())
    doxygen_data.set('PACKAGE_VERSION', meson.project_version())
    doxygen_data.set('INPUT', ' '.join(doxygen_input))
    doxygen_data.set('OUTPUT_DIRECTORY', meson.build_root())
    doxyfile = configure_file(
        input: 'doc/Doxyfile.in',
        output: 'Doxyfile',
        configuration: doxygen_data,
    )
    # TODO: Meson should provide this.
    docdir = join_paths(get_option('datadir'), 'doc', meson.project_name())
    custom_target(
        'doc',
        input: [doxyfile] + doxygen_input,
        output: 'html',
        command: [doxygen_wrapper, doxygen.path(), join_paths(meson.build_root(), 'Doxyfile'), meson.source_root()],
        install: true,
        install_dir: docdir,
        build_by_default: true,
    )
endif
