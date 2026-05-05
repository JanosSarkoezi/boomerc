project('boomer', 'c',
  version : '0.0.1',
  default_options : ['warning_level=3'])

cc = meson.get_compiler('c')

x11_dep = dependency('x11')
xrandr_dep = dependency('xrandr')
gl_dep = dependency('gl')
m_dep = cc.find_library('m', required : false)

sources = [
  'src/main.c',
  'src/la.c',
  'src/config.c',
  'src/navigation.c',
  'src/screenshot.c',
  'src/render.c'
]

executable('boomer',
           sources,
           dependencies : [x11_dep, xrandr_dep, gl_dep, m_dep],
           install : true)
