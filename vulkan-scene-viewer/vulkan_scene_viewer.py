from PySide2 import (QtGui, QtCore)
import vk_renderer

class SimpleVulkanApplication(QtGui.QWindow):

    def __init__(self):
        super(SimpleVulkanApplication, self).__init__()

        self.setWidth(1280)
        self.setHeight(720)

        self.setTitle("Vulkan Python - PySide2")

        self.timer = QtCore.QTimer(self)
        self.timer.timeout.connect(self.render)

        self.initialize()
        self.timer.start()

    def __del__(self):
        vk_renderer.cleanup()
        self.destroy()

    def initialize(self):
        vk_renderer.initialize(self.winId(), self.width(), self.height())

    def render(self):
        if not self.isExposed():
            return
        vk_renderer.render()


if __name__ == '__main__':
    import sys

    app = QtGui.QGuiApplication(sys.argv)

    win = SimpleVulkanApplication()
    win.show()

    def cleanup():
        global win
        del win
    
    app.aboutToQuit.connect(cleanup)

    sys.exit(app.exec_())