#-------------------------------------------------
#
# Project created by QtCreator 2018-02-03T01:59:53
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 4): DEFINES += ADS_NAMESPACE_ENABLED

TARGET = untitled1
TEMPLATE = app

CONFIG += static

release:DESTDIR = release
release:OBJECTS_DIR = release/.tmp
release:MOC_DIR = release/.tmp
release:RCC_DIR = release/.tmp
release:UI_DIR = release/.tmp

debug:DESTDIR = debug
debug:OBJECTS_DIR = debug/.tmp
debug:MOC_DIR = debug/.tmp
debug:RCC_DIR = debug/.tmp
debug:UI_DIR = debug/.tmp

SOURCES += main.cpp\
        mainwindow.cpp \
    dialog.cpp \
    consolewidget.cpp \
    scenetreewidget.cpp \
    icontitlewidget.cpp \
    EngineGlobal.cpp \
    renderwidget.cpp \
    projectview.cpp \
    propertieswidget.cpp \
    editorglobal.cpp \
    about.cpp \
    manipulators/ManipulatorTranslator.cpp \
    manipulators/ManipulatorUtils.cpp \
    manipulators/ManipulatorBase.cpp

HEADERS  += mainwindow.h \
    dialog.h \
    consolewidget.h \
    scenetreewidget.h \
    icontitlewidget.h \
    EngineGlobal.h \
    renderwidget.h \
    projectview.h \
    propertieswidget.h \
    editorglobal.h \
    myspinbox.h \
    mylineedit.h \
    about.h \
    common.h \
    manipulators/ManipulatorTranslator.h \
    manipulators/ManipulatorUtils.h \
    manipulators/ManipulatorBase.h

FORMS    += mainwindow.ui \
    dialog.ui \
    consolewidget.ui \
    scenetreewidget.ui \
    dd3d11widget.ui \
    projectview.ui \
    propertieswidget.ui \
    about.ui

RESOURCES += \
    myresources.qrc

DISTFILES += \
    render master2.png

INCLUDEPATH += "$$PWD/../RenderMaster/include"

win32:LIBS += -lOle32

CONFIG( debug, debug|release ) {
    # debug
    win32:LIBS += "$$PWD/advanceddockingsystem/debug/AdvancedDockingSystem.lib"
} else {
    # release
    win32:LIBS += "$$PWD/advanceddockingsystem/release/AdvancedDockingSystem.lib"
}


INCLUDEPATH += "$$PWD/advanceddockingsystemm/include"
DEPENDPATH += "$$PWD/advanceddockingsystem/include"

# force relink
PRE_TARGETDEPS += $$PWD/advanceddockingsystem/debug/AdvancedDockingSystem.lib
