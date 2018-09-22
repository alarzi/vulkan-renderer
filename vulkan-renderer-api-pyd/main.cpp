#include <pybind11/pybind11.h>
#include "../Renderer/VulkanRenderer.h"

void initialize(int hWnd, int width, int height) {
	HINSTANCE hInstance = (HINSTANCE)::GetModuleHandle(NULL);
	VulkanRenderer::get_instance()->initialize(hInstance, (HWND)hWnd, width, height);
}

void render() {
	VulkanRenderer::get_instance()->render();
}

void cleanup() {
	VulkanRenderer::get_instance()->shutdown();
}

PYBIND11_MODULE(vk_renderer, m) {

	m.def("initialize", &initialize);
	m.def("render", &render);
	m.def("cleanup", &cleanup);

#ifdef VERSION_INFO
	m.attr("__version__") = VERSION_INFO;
#else
	m.attr("__version__") = "dev";
#endif
}