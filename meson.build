project('mini-settings-scripts', 'c')

c_args = []
if get_option('default_config_path') != ''
  c_args += ['-DDEFAULT_CONFIG_PATH="@0@"'.format(get_option('default_config_path'))]
endif

executable(get_option('get_exe_name'), ['mini_settings_get_main.c', 'mini_settings_get.c', 'mini_settings_read_file.c'], install: true, c_args: c_args)
executable(get_option('set_exe_name'), ['mini_settings_set_main.c', 'mini_settings_set.c', 'mini_settings_read_file.c', 'mini_settings_stringbuf.c'], install: true, c_args: c_args)
