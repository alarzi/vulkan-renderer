#include <pybind11/pybind11.h>
#include "System/VulkanExports.h"

PYBIND11_MODULE(vk_py_renderer, m) {
	m.def("initialize", [](int hWnd, int width, int height) { vk_initialize(NULL, (HWND)hWnd, width, height); });
	m.def("render", []() { vk_render(); });
	m.def("cleanup", []() { vk_shutdown(); });

#ifdef VERSION_INFO
	m.attr("__version__") = VERSION_INFO;
#else
	m.attr("__version__") = "dev";
#endif
}