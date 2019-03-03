#-------------------------------------------------
#
# Project created by QtCreator 2019-03-03T11:12:06
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ChannelForward
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
    ChannelUnitForward.cpp \
    comm_socket.cpp \
    epoll_wrapper.cpp \
    inifile.cpp \
    main.cpp \
    tcp_socket.cpp \
    udp_socket.cpp \
    UDPInterface.cpp \
    utils.cpp \
        main.cpp \
        mainwindow.cpp

HEADERS += \
    ChannelUnitForward.h \
    comm_socket.h \
    common_defs.h \
    epoll_wrapper.h \
    inifile.h \
    tcp_socket.h \
    udp_socket.h \
    UDPInterface.h \
    unistd.h \
    utils.h \
        mainwindow.h

FORMS += \
        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
