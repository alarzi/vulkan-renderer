#include <pybind11/pybind11.h>

#include "Renderer/VulkanRenderer.h"
#include "System/VulkanExports.h"

namespace py = pybind11;

PYBIND11_MODULE(vk_py_renderer, m) {
	//m.def("initialize", [](int hWnd, int width, int height) { vk_initialize(NULL, (HWND)hWnd, width, height); });
	//m.def("resize", [](int width, int height) { vk_resize(width, height); });
	//m.def("render", []() { vk_render(); });
	//m.def("cleanup", []() { vk_shutdown(); });

	py::class_<VulkanRenderer, std::shared_ptr<VulkanRenderer>>(m, "VulkanRenderer")
		.def(py::init<>())
		.def("initialize", &VulkanRenderer::initialize_)
		.def("resize", &VulkanRenderer::resize)
		.def("render", &VulkanRenderer::render)
		.def("update", &VulkanRenderer::update)
		.def("cleanup", &VulkanRenderer::shutdown)
		.def_readwrite("is_paused", &VulkanRenderer::is_paused);

#ifdef VERSION_INFO
	m.attr("__version__") = VERSION_INFO;
#else
	m.attr("__version__") = "dev";
#endif
}

// https://pybind11.readthedocs.io/en/stable/basics.html
// https://developer.lsst.io/v/u-ktl-debug-fix/coding/python_wrappers_for_cpp_with_pybind11.html

// http://www.benjack.io/2017/06/12/python-cpp-tests.html
// https://pybind11.readthedocs.io/en/stable/classes.html