import os, sys

from distutils.core import setup, Extension
from distutils import sysconfig

cpp_args = ['-std=c++11', '-stdlib=libc++', '-mmacosx-version-min=10.7']

vk_module = Extension(
    'vk_py_renderer', sources = ['module.cpp'],
    include_dirs=['pybind11/include', 'D:/Documents/Vulkan/Projects/vulkan-renderer/vulkan-renderer-core'],
    library_dirs=['D:/Documents/Vulkan/Projects/vulkan-renderer/Debug'],
    libraries=['vk_renderer'],
    language='c++',
    extra_compile_args = cpp_args,
    )

setup(
    name = 'vk_py_renderer',
    version = '1.0',    

    description = '',
    ext_modules = [vk_module],
)