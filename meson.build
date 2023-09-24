project('goide', 'c',
	version: '1.0.0',
	default_options: ['c_std=gnu2x', 'warning_level=3']
)

builder_plugin_dir = get_option('datadir') / 'gnome-builder' / 'plugins'

goide_res = import('gnome').compile_resources('goide', 'goide.gresource.xml',
	gresource_bundle: true,
	install: true,
	install_dir: builder_plugin_dir
)

goide_plugin = shared_module('goide', 'goide.c', 'goide-tree-addin.c', 'goide-new-class-diag.c', goide_res,
	c_args: ['-DG_LOG_DOMAIN="GoIde"', '-Wno-pedantic', '-Wno-unused-parameter'],
	dependencies: [
		dependency('gnome-builder-45.0'),
		dependency('libdex-1'),
		dependency('libadwaita-1'),
	],
	install: true,
	install_dir: builder_plugin_dir
)

install_data('gobject-ide-extensions.plugin', install_dir: builder_plugin_dir)