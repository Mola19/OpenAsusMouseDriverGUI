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
		"/usr/include/openasusmousedriver/"
}

SOURCES += \
    devicepage.cpp \
    directzone.cpp \
    lightingzone.cpp \
    main.cpp \
    mainwindow.cpp \
    statsthread.cpp

HEADERS += \
    cache.h \
    devicepage.h \
    directzone.h \
    lightingzone.h \
    mainwindow.h \
    statsthread.h

FORMS += \
    devicepage.ui \
    directzone.ui \
    lightingzone.ui \
    mainwindow.ui

win32 {
    LIBS +=                                                                                         \
        -L"$$(OpenAsusMouseDriver)/lib/" -L"$$(OpenAsusMouseDriver)/bin/" \
        -l"hidapi" -l"openasusmousedriver"
}

unix {
    LIBS +=                                                                                         \
        -L"/usr/lib/" -l"openasusmousedriver"

    packagesExist(hidapi-hidraw) {
        HIDAPI_HIDRAW_VERSION = $$system($$PKG_CONFIG --modversion hidapi-hidraw)
        if (versionAtLeast(HIDAPI_HIDRAW_VERSION, "0.10.1")) {
            LIBS += -lhidapi-hidraw
        } else {
            packagesExist(hidapi-libusb) {
                LIBS += -lhidapi-libusb
            } else {
                LIBS += -lhidapi
            }
        }
    } else {
        packagesExist(hidapi-libusb) {
            LIBS += -lhidapi-libusb
        } else {
            LIBS += -lhidapi
        }
    }
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
