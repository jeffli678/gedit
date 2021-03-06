libfilebrowser_public_h = files(
  'gedit-file-bookmarks-store.h',
  'gedit-file-browser-error.h',
  'gedit-file-browser-store.h',
  'gedit-file-browser-view.h',
  'gedit-file-browser-widget.h',
  'gedit-file-browser-utils.h',
  'gedit-file-browser-plugin.h',
  'gedit-file-browser-messages.h',
)

libfilebrowser_sources = files(
  'gedit-file-bookmarks-store.c',
  'gedit-file-browser-messages.c',
  'gedit-file-browser-plugin.c',
  'gedit-file-browser-store.c',
  'gedit-file-browser-utils.c',
  'gedit-file-browser-view.c',
  'gedit-file-browser-widget.c',
)

libfilebrowser_deps = [
  libgedit_dep,
]

libfilebrowser_c_args = [
  '-DHAVE_CONFIG_H',
]

subdir('messages')

libfilebrowser_register_enums = gnome.mkenums(
  'gedit-file-browser-enum-register',
  sources: libfilebrowser_public_h,
  c_template: 'gedit-file-browser-enum-register.c.template',
)

libfilebrowser_type_enums = gnome.mkenums(
  'gedit-file-browser-enum-types',
  depends : [libfilebrowser_register_enums],
  sources: libfilebrowser_public_h,
  h_template: 'gedit-file-browser-enum-types.h.template',
  c_template: 'gedit-file-browser-enum-types-stage1.c.template',
)

# cat won't work on Windows so this
# will need to be reimplemented as a script
cat = find_program('cat')

# Combine the 2 C mkenums templates together before compiling
libfilebrowser_enums_c = custom_target('libfilebrowser_enums_c',
  input: [libfilebrowser_type_enums.get(0),
          libfilebrowser_register_enums],
  output: 'gedit-file-browser-enum-types.c',
  command: [cat, '@INPUT0@', '@INPUT1@'],
  # redirects the command output since we can't use >> here
  capture: true,
)

libfilebrowser_sources += [
  libfilebrowser_enums_c,
]

subdir('resources')

libfilebrowser_sha = shared_module(
  'filebrowser',
  sources: libfilebrowser_sources,
  include_directories: rootdir,
  dependencies: libfilebrowser_deps,
  c_args: libfilebrowser_c_args,
  install: true,
  install_dir: join_paths(
    pkglibdir,
    'plugins',
  )
)

# FIXME: https://github.com/mesonbuild/meson/issues/1687
custom_target(
  'org.gnome.gedit.plugins.filebrowser.enums.xml',
  input : libfilebrowser_sources + libfilebrowser_public_h,
  output: 'org.gnome.gedit.plugins.filebrowser.enums.xml',
  capture: true,
  command: [
    'glib-mkenums',
    '--comments', '<!-- @comment@ -->',
    '--fhead', '<schemalist>',
    '--vhead', '  <@type@ id="org.gnome.gedit.plugins.filebrowser.@EnumName@">',
    '--vprod', '    <value nick="@valuenick@" value="@valuenum@"/>',
    '--vtail', '  </@type@>',
    '--ftail', '</schemalist>',
    '@INPUT@'
  ],
  install: true,
  install_dir: join_paths(
    glibdir,
    'schemas',
  )
)

configure_file(
  input: 'org.gnome.gedit.plugins.filebrowser.gschema.xml.in',
  output: 'org.gnome.gedit.plugins.filebrowser.gschema.xml',
  configuration: gschema_xml,
  install: true,
  install_dir: join_paths(
    glibdir,
    'schemas',
  )
)

custom_target(
  'filebrowser.plugin',
  input: 'filebrowser.plugin.desktop.in',
  output: 'filebrowser.plugin',
  command: msgfmt_plugin_cmd,
  install: true,
  install_dir: join_paths(
    pkglibdir,
    'plugins',
  )
)
