from distutils.core import setup, Extension
import numpy

module1 = Extension('ringbuffer',
                    include_dirs=[numpy.get_include()],
                    sources = ['c_src/python_bindings.c', 'c_src/ringbuffer.c'])

setup (name = 'ringbuffer',
       version = '1.0',
       description = 'C implemented ringbuffer for python',
       scripts=['scripts/read_stdin'],
       install_requires=[
            'numpy',
            "humanfriendly"
       ],
       requires=[
           "numpy",
           "humanfriendly"
       ],
       ext_modules = [module1],
)