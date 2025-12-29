
QT += core gui widgets sql mqtt
QMAKE_RPATHDIR += /opt/firefly_qt5.15/lib
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    basicparawidget.cpp \
    channelbasicpara.cpp \
    channelmodelselect.cpp \
    channelparaconifg.cpp \
    channelselect.cpp \
    channelsimumain.cpp \
    configmanager.cpp \
    databasemanager.cpp \
    datamanager.cpp \
    main.cpp \
    mainwindow.cpp \
    matrixwidget.cpp \
    modelselect.cpp \
    mqttclient.cpp \
    mqttmessageparser.cpp \
    multipathpara.cpp \
    pageindicator.cpp \
    pagewidget.cpp \
    parasetting.cpp \
    screenadapter.cpp \
    screensaver.cpp \
    settingmanager.cpp \
    simulistview.cpp \
    subwindow.cpp \
    swipestackedwidget.cpp \
    systemsetting.cpp

HEADERS += \
    basicparawidget.h \
    channelbasicpara.h \
    channelmodelselect.h \
    channelparaconifg.h \
    channelselect.h \
    channelsimumain.h \
    configmanager.h \
    databasemanager.h \
    datamanager.h \
    mainwindow.h \
    matrixwidget.h \
    modelselect.h \
    mqttclient.h \
    mqttmessageparser.h \
    multipathpara.h \
    pageindicator.h \
    pagewidget.h \
    parasetting.h \
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

FORMS += \
    channelsimumain.ui
