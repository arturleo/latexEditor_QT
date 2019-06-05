QT += quick quickcontrols2 qml sql widgets websockets
CONFIG += c++11

include(lib/diff-match-patch/diff_match_patch.pro)
include(lib/poppler-qml/poppler-qml.pri)
# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Refer to the documentation for the
# deprecated API to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        httprequest.cpp \
        latexbasichighlighter.cpp \
        latexclient.cpp \
        latexhandler.cpp \
        main.cpp \
        texfontsize.cpp \
        texsetting.cpp

RESOURCES += latexEditor_QT.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    httprequest.h \
    latexbasichighlighter.h \
    latexclient.h \
    latexhandler.h \
	datatypes.h \
    texfontsize.h \
    texsetting.h

unix|win32: LIBS += -L"$$_PRO_FILE_PWD_/lib/poppler-64bit-libs/lib/" -lpoppler-qt5