QT       += core gui serialport network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES += MY_DELAY

INCLUDEPATH += ../../src/

SOURCES += \
    ../../src/danish.c \
    ../../src/danish_link.c \
    binaryinserter.cpp \
    delay.c \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    binaryinserter.h \
    danish_conf.h \
    delay.h \
    log.h \
    mainwindow.h

FORMS += \
    binaryinserter.ui \
    mainwindow.ui

TRANSLATIONS += \
    danish-monitor_az_IR.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
