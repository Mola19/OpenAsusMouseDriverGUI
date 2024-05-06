QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

win32 {
    INCLUDEPATH += \
        "$$(OpenAsusMouseDriver)/include/" \
}

unix {
	INCLUDEPATH += \
		"/usr/include/hidapi/" \
		"/usr/include/openasusmicedriver/"
}

SOURCES += \
    devicepage.cpp \
    lightingzone.cpp \
    main.cpp \
    mainwindow.cpp \
    statsthread.cpp

HEADERS += \
    devicepage.h \
    lightingzone.h \
    mainwindow.h \
    statsthread.h

FORMS += \
    devicepage.ui \
    lightingzone.ui \
    mainwindow.ui

win32 {
    LIBS +=                                                                                         \
        -L"$$(OpenAsusMouseDriver)/lib/" -L"$$(OpenAsusMouseDriver)/bin/" \
        -l"hidapi" -l"openasusmousedriver"
}

unix {
    LIBS +=                                                                                         \
        -L"/usr/lib/" -l"hidapi"      \
        -L"/usr/lib/" -l"openasusmousedriver"
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
