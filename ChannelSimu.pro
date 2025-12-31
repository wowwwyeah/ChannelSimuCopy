
QT += core gui widgets sql
QMAKE_RPATHDIR += /opt/firefly_qt5.15/lib
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

#ARM交叉编译配置 MQTT
QT += mqtt

# # GCC构建配置 - 手动链接MQTT
# QT_INSTALL_DIR = /home/dlj/Qt/5.15.2
# QT_QMAKE_DIR = $$QT_INSTALL_DIR/gcc_64/bin/
# INCLUDEPATH += $$QT_INSTALL_DIR/gcc_64/include/QtMqtt
# LIBS += -L$$QT_INSTALL_DIR/gcc_64/lib -lQt5Mqtt


#定义USE_TOUCH_EVENT宏以支持触摸事件
DEFINES += USE_TOUCH_EVENT

# 定义USE_FPGA_TEST宏以支持FPGA测试功能
# DEFINES += USE_FPGA_TEST

SOURCES += \
    PttMonitorThread.cpp \
    RadioChannelManager.cpp \
    channelbasicpara.cpp \
    channelmodelselect.cpp \
    channelparaconifg.cpp \
    channelselect.cpp \
    configmanager.cpp \
    databasemanager.cpp \
    datamanager.cpp \
    iohandler.cpp \
    fpga_driver.cpp \
    main.cpp \
    mainwindow.cpp \
    matrixwidget.cpp \
    mqttclient.cpp \
    mqttmessageparser.cpp \
    multipathpara.cpp \
    pageindicator.cpp \
    screenadapter.cpp \
    screensaver.cpp \
    settingmanager.cpp \
    simulistview.cpp \
    subwindow.cpp \
    swipestackedwidget.cpp \
    systemsetting.cpp

HEADERS += \
    PttMonitorThread.h \
    RadioChannelManager.h \
    channel_utils.h \
    channelbasicpara.h \
    channelmodelselect.h \
    channelparaconifg.h \
    channelselect.h \
    configmanager.h \
    databasemanager.h \
    datamanager.h \
    iohandler.h \
    fpga_driver.h \
    mainwindow.h \
    matrixwidget.h \
    mqttclient.h \
    mqttmessageparser.h \
    multipathpara.h \
    pageindicator.h \
    screenadapter.h \
    screensaver.h \
    settingmanager.h \
    simulistview.h \
    subwindow.h \
    swipestackedwidget.h \
    systemsetting.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

DISTFILES += \
    res/image/arrow_down.png \
    res/image/arrow_left.png \
    res/image/arrow_right.png \
    res/image/arrow_up.png \
    res/image/cancel.png \
    res/image/circle_plus.png \
    res/image/circle_remove.png \
    res/image/logo.png

