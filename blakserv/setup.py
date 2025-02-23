from setuptools import setup, Extension
import sysconfig

python_include_dir = sysconfig.get_path('include')
python_lib_dir = sysconfig.get_config_var('LIBDIR')

# Ensure LIBDIR is valid
if not python_lib_dir:
    python_lib_dir = "C:\\Users\\toko\\AppData\\Local\\Programs\\Python\\Python310\\libs"  # Adjust for your system

module = Extension(
    'api_dispatch',
    sources=['api_dispatch.c', 'apifn.c'],
    include_dirs=[
        '.', 
        'util', 
        '../include', 
        python_include_dir,
        'path/to/other/includes'
    ],
    library_dirs=[python_lib_dir],
    libraries=['python310'],  # Adjust if necessary
    define_macros=[('SOME_MACRO', '1'), ('ANOTHER_MACRO', '1')],
    extra_compile_args=['/DWIN32', '/D_WINDOWS', '/W3'],
    language='c'
)

setup(
    name='api_dispatch',
    version='1.0',
    description='Python C extension for Blakserv API',
    ext_modules=[module]
)
