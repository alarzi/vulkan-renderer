import sys
import time
from PyQt5 import QtCore, QtGui, QtWidgets
from lib import vk_py_renderer

sys.path.append('lib\QtProperty')
sys.path.append('lib\libqt5')

from pyqtcore import QList
from qtvariantproperty import QtVariantEditorFactory, QtVariantPropertyManager
from qttreepropertybrowser import QtTreePropertyBrowser

from qtpropertymanager import (
    QtBoolPropertyManager,
    QtDoublePropertyManager, 
    QtStringPropertyManager, 
    QtColorPropertyManager, 
    QtFontPropertyManager, 
    QtPointPropertyManager,
    QtSizePropertyManager
)
from qteditorfactory import (
    QtDoubleSpinBoxFactory, 
    QtCheckBoxFactory, 
    QtSpinBoxFactory, 
    QtLineEditFactory, 
    QtEnumEditorFactory
)

import style

class VulkanWindow(QtGui.QWindow):

    def __init__(self, parent=None):
        super(VulkanWindow, self).__init__()
        self.vk_renderer = vk_py_renderer.VulkanRenderer()
        self.timer = QtCore.QTimer(self)
        self.timer.timeout.connect(self.render)

        self.fps_timer = time.perf_counter() * 1000
        self.last_elapsed_time =0
        self.fps = 0

    def __del__(self):
        cleanup()

    def initialize(self):
        self.vk_renderer.initialize(self.winId(), self.width(), self.height())
        self.timer.start()

    def render(self):
        self.vk_renderer.render()
        self.vk_renderer.update(self.last_elapsed_time)

        self.fps += 1
        if (time.perf_counter() * 1000 - self.fps_timer >= 1000):
            self.fps_timer = time.perf_counter() * 1000
            #print("FPS: %s" % self.fps)
            self.fps = 0
        self.last_elapsed_time = time.perf_counter()

    def cleanup(self):
        self.vk_renderer.cleanup()

    def resizeEvent(self, event):
        self.vk_renderer.resize(self.width(), self.height())

class MainWindow(QtWidgets.QMainWindow):

    def __init__(self):
        super(MainWindow, self).__init__()

        self.setGeometry(0, 0, 1280, 780)

        # Create actions and menus
        self.createMenu()
        self.createToolBar()

        # Create status bar
        self.statusBar().showMessage("Ready")

        # Vulkan widget
        self.vulkanWindow = VulkanWindow(self)
        self.vulkanWindowWidget = QtWidgets.QWidget.createWindowContainer(self.vulkanWindow)
        self.vulkanWindow.initialize()

        # Stop rendering while moving the window
        self.moveEventTimer = QtCore.QTimer(self)
        self.moveEventTimer.timeout.connect(self.moveEventDone)

        # Dock layout
        dock = QtWidgets.QDockWidget()
        dock.setAllowedAreas(QtCore.Qt.LeftDockWidgetArea | QtCore.Qt.RightDockWidgetArea)
        self.addDockWidget(QtCore.Qt.LeftDockWidgetArea, dock)

        # Property editor
        self.variantManager = QtVariantPropertyManager(self)
        self.variantManager.valueChangedSignal.connect(self.valueChanged)
        topItem = self.variantManager.addProperty(QtVariantPropertyManager.groupTypeId(), "Group Property")
        item = self.variantManager.addProperty(QtCore.QVariant.Bool, "Pause?")
        item.setValue(False)
        topItem.addSubProperty(item)
        self.variantFactory = QtVariantEditorFactory()
        self.propertyEditor = QtTreePropertyBrowser(dock)
        self.propertyEditor.setFactoryForManager(self.variantManager, self.variantFactory)
        self.propertyEditor.addProperty(topItem)
        self.propertyEditor.setPropertiesWithoutValueMarked(True)
        self.propertyEditor.setRootIsDecorated(False)
        
        dock.setWidget(self.propertyEditor)

        # Vulkan viewport
        dock = QtWidgets.QDockWidget()
        dock.setWidget(self.vulkanWindowWidget)

        # Box layout
        #layout = QtWidgets.QVBoxLayout()
        #layout.addWidget(self.button)
        #layout.addWidget(self.vulkanWindowWidget)
        #layout.addWidget(dock)
        
        
        #mainWidget = QtWidgets.QWidget()
        #mainWidget.setLayout(layout)

        #self.setCentralWidget(mainWidget)
        self.setCentralWidget(self.vulkanWindowWidget)

        #self.showMaximized()


    def __del__(self):
        del self.variantManager
        del self.variantFactory
        del self.variantEditor
        self.destroy()

    def valueChanged(self, property, value):
        tp = type(value)
        if tp == bool:
             self.vulkanWindow.vk_renderer.is_paused = value

    def closeEvent(self, event):
        self.vulkanWindow.cleanup()
        del self.vulkanWindow

    def moveEvent(self, event):
        self.moveEventTimer.start(500)
        self.vulkanWindow.timer.stop()

    def moveEventDone(self):
        self.vulkanWindow.timer.start()

    def createMenu(self):
        fileMenu = self.menuBar().addMenu("&File")
        fileMenu.addAction("Exit", self.exit)

    def createToolBar(self):
        exitAction = QtWidgets.QAction(QtGui.QIcon(":/images/open.png"), "&Open...", self)
        exitAction.setStatusTip("Open an existing file")
        exitAction.triggered.connect(self.exit)
        self.fileToolBar = self.addToolBar("File")
        self.fileToolBar.addAction(exitAction)

    def exit(self):
        print("exit")

if __name__ == '__main__':

    import sys
    app = QtWidgets.QApplication(sys.argv)
    app.setAttribute(QtCore.Qt.AA_EnableHighDpiScaling)

    style.set_app_style(app)
  
    win = MainWindow()
    win.show()

    def cleanup():
        print("exit")
        global win
        del win

    app.aboutToQuit.connect(cleanup)
    sys.exit(app.exec_())
