from distutils.core import setup, Extension
import numpy

module1 = Extension('ringbuffer',
                    include_dirs=[numpy.get_include()],
                    sources = ['c_src/python_bindings.c', 'c_src/ringbuffer.c'])

setup (name = 'ringbuffer',
       version = '1.0',
       description = 'This is a demo package',
       ext_modules = [module1])