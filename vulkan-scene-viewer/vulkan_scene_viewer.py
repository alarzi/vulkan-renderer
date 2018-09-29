from PySide2 import QtCore, QtGui, QtWidgets
from lib import vk_py_renderer

class VulkanWindow(QtGui.QWindow):

    def __init__(self):
        super(VulkanWindow, self).__init__()
        self.timer = QtCore.QTimer(self)
        self.timer.timeout.connect(self.render)
        self.timer.start()

    def __del__(self):
        vk_py_renderer.cleanup()

    def initialize(self):
        vk_py_renderer.initialize(self.winId(), self.width(), self.height())

    def render(self):
        if not self.isExposed():
            return
        vk_py_renderer.render()

    def resizeEvent(self, event):
        vk_py_renderer.resize(self.width(), self.height())

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
        self.vulkanWindow = VulkanWindow()
        self.vulkanWindowWidget = QtWidgets.QWidget.createWindowContainer(self.vulkanWindow)

        # Stop rendering while moveing the window
        self.moveEventTimer = QtCore.QTimer(self)
        self.moveEventTimer.timeout.connect(self.moveEventDone)

        # Initialization button
        self.initialize()
        self.button = QtWidgets.QPushButton("Initialize Vulkan")
        self.button.clicked.connect(self.initialize)

        #dock = QtWidgets.QDockWidget()
        #dock.setAllowedAreas(QtCore.Qt.LeftDockWidgetArea | QtCore.Qt.RightDockWidgetArea)
        #self.customerList = QtWidgets.QListWidget(dock)
        #self.customerList.addItems((
        #    "John Doe, Harmony Enterprises, 12 Lakeside, Ambleton",
        #    "Jane Doe, Memorabilia, 23 Watersedge, Beaton",
        #    "Tammy Shea, Tiblanka, 38 Sea Views, Carlton",
        #    "Tim Sheen, Caraba Gifts, 48 Ocean Way, Deal",
        #    "Sol Harvey, Chicos Coffee, 53 New Springs, Eccleston",
        #    "Sally Hobart, Tiroli Tea, 67 Long River, Fedula"))
        #dock.setWidget(self.customerList)
        #self.addDockWidget(QtCore.Qt.LeftDockWidgetArea, dock)

        #dock = QtWidgets.QDockWidget()
        #dock.setWidget(self.vulkanWindowWidget)
        #self.addDockWidget(QtCore.Qt.RightDockWidgetArea, dock)

        #self.setCentralWidget(self.vulkanWindowWidget)
        
        # Main layout
        layout = QtWidgets.QVBoxLayout()
        layout.addWidget(self.button)
        layout.addWidget(self.vulkanWindowWidget)

        mainWidget = QtWidgets.QWidget()
        mainWidget.setLayout(layout)
        self.setCentralWidget(mainWidget)

        self.showMaximized()

    def __del__(self):
        self.destroy()

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

    def initialize(self):
        self.vulkanWindow.initialize()

    def exit(self):
        print("exit")

if __name__ == '__main__':
    import sys
    app = QtWidgets.QApplication(sys.argv)
    app.setAttribute(QtCore.Qt.AA_EnableHighDpiScaling)

    base_palette = QtGui.QPalette()

    HIGHLIGHT_COLOR = QtGui.QColor(103, 141, 178)
    BRIGHTNESS_SPREAD = 2.5

    BRIGHT_COLOR = QtGui.QColor(200, 200, 200)
    LIGHT_COLOR = QtGui.QColor(100, 100, 100)
    DARK_COLOR = QtGui.QColor(42, 42, 42)
    MID_COLOR = QtGui.QColor(68, 68, 68)
    MID_LIGHT_COLOR = QtGui.QColor(84, 84, 84)
    SHADOW_COLOR = QtGui.QColor(21, 21, 21)

    BASE_COLOR = MID_COLOR
    TEXT_COLOR = BRIGHT_COLOR
    DISABLED_BUTTON_COLOR = QtGui.QColor(78, 78, 78)
    DISABLED_TEXT_COLOR = QtGui.QColor(128, 128, 128)
    ALTERNATE_BASE_COLOR = QtGui.QColor(46, 46, 46)

    #if app.lightness(BASE_COLOR) > 0.5:
    #    SPREAD = 100 / BRIGHTNESS_SPREAD
    #else:
    #    SPREAD = 100 * BRIGHTNESS_SPREAD

    #if app.lightness(HIGHLIGHT_COLOR) > 0.6:
    #    HIGHLIGHTEDTEXT_COLOR= BASE_COLOR.darker(SPREAD*2)
    #else:
    #    HIGHLIGHTEDTEXT_COLOR= BASE_COLOR.lighter(SPREAD*2)

    SPREAD = 100 / BRIGHTNESS_SPREAD
    #SPREAD = 100 * BRIGHTNESS_SPREAD
    HIGHLIGHTEDTEXT_COLOR= BASE_COLOR.darker(SPREAD*2)
    #HIGHLIGHTEDTEXT_COLOR= BASE_COLOR.lighter(SPREAD*2)

    base_palette.setBrush(QtGui.QPalette.Window, QtGui.QBrush(MID_COLOR))
    base_palette.setBrush(QtGui.QPalette.WindowText, QtGui.QBrush(TEXT_COLOR))
    base_palette.setBrush(QtGui.QPalette.Foreground, QtGui.QBrush(BRIGHT_COLOR))
    base_palette.setBrush(QtGui.QPalette.Base, QtGui.QBrush(DARK_COLOR))
    base_palette.setBrush(QtGui.QPalette.AlternateBase, QtGui.QBrush(ALTERNATE_BASE_COLOR))
    base_palette.setBrush(QtGui.QPalette.ToolTipBase, QtGui.QBrush(BASE_COLOR))
    base_palette.setBrush(QtGui.QPalette.ToolTipText, QtGui.QBrush(TEXT_COLOR))

    base_palette.setBrush(QtGui.QPalette.Text, QtGui.QBrush(TEXT_COLOR))
    base_palette.setBrush(QtGui.QPalette.Disabled, QtGui.QPalette.Text, QtGui.QBrush(DISABLED_TEXT_COLOR))

    base_palette.setBrush(QtGui.QPalette.Button, QtGui.QBrush(LIGHT_COLOR))
    base_palette.setBrush(QtGui.QPalette.Disabled, QtGui.QPalette.Button, QtGui.QBrush(DISABLED_BUTTON_COLOR))
    base_palette.setBrush(QtGui.QPalette.ButtonText, QtGui.QBrush(TEXT_COLOR))
    base_palette.setBrush(QtGui.QPalette.Disabled, QtGui.QPalette.ButtonText, QtGui.QBrush(DISABLED_TEXT_COLOR))
    base_palette.setBrush(QtGui.QPalette.BrightText, QtGui.QBrush(TEXT_COLOR))
    base_palette.setBrush(QtGui.QPalette.Disabled, QtGui.QPalette.BrightText, QtGui.QBrush(DISABLED_TEXT_COLOR))

    base_palette.setBrush(QtGui.QPalette.Light, QtGui.QBrush(LIGHT_COLOR))
    base_palette.setBrush(QtGui.QPalette.Midlight, QtGui.QBrush(MID_LIGHT_COLOR))
    base_palette.setBrush(QtGui.QPalette.Mid, QtGui.QBrush(MID_COLOR))
    base_palette.setBrush(QtGui.QPalette.Dark, QtGui.QBrush(DARK_COLOR))
    base_palette.setBrush(QtGui.QPalette.Shadow, QtGui.QBrush(SHADOW_COLOR))

    base_palette.setBrush(QtGui.QPalette.Highlight, QtGui.QBrush(HIGHLIGHT_COLOR))
    base_palette.setBrush(QtGui.QPalette.HighlightedText, QtGui.QBrush(HIGHLIGHTEDTEXT_COLOR))

    # Setup additional palettes for QTabBar and QTabWidget to look more like
    # maya.
    tab_palette = QtGui.QPalette(base_palette)
    tab_palette.setBrush(QtGui.QPalette.Window, QtGui.QBrush(LIGHT_COLOR))
    tab_palette.setBrush(QtGui.QPalette.Button, QtGui.QBrush(MID_COLOR))

    widget_palettes = {}
    widget_palettes["QTabBar"] = tab_palette
    widget_palettes["QTabWidget"] = tab_palette

    app.setStyle("Fusion")
    app.setPalette(base_palette)
    for name, palette in widget_palettes.items():
        app.setPalette(palette, name)
  
    win = MainWindow()
    win.show()
    def cleanup():
        global win
        del win
    app.aboutToQuit.connect(cleanup)
    sys.exit(app.exec_())
