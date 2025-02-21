from setuptools import setup, Extension
import sysconfig

# Get the include path for Python headers and the library path for Python libraries
python_include_dir = sysconfig.get_path('include')
python_lib_dir = sysconfig.get_config_var('LIBDIR')

module = Extension(
    'apifn',
    sources=['apifn.c'],
    include_dirs=['.', 'util', '../include', python_include_dir],  # Add the directories where header files are located
    library_dirs=[python_lib_dir],  # Add the directory where Python libraries are located
    libraries=['python310'],  # Link against the Python library (adjust the version as needed)
    extra_compile_args=['/TC'],  # Force C mode
    language='c'  # Make sure it's compiled as C
)

setup(
    name='apifn',
    version='1.0',
    description='Python C extension for Blakserv API',
    ext_modules=[module]
)