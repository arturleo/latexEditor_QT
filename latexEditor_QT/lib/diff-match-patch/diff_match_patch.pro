#QT += sql xml network
INCLUDEPATH += $$PWD
TEMPLATE = app
CONFIG += qt debug_and_release

mac {
  CONFIG -= app_bundle
}

# don't embed the manifest for now (doesn't work :( )
#CONFIG -= embed_manifest_exe 

FORMS =

HEADERS += \
    $$PWD/diff_match_patch.h \
    #$$PWD/diff_match_patch_test.h

SOURCES += \
    $$PWD/diff_match_patch.cpp \
    #$$PWD/diff_match_patch_test.cpp

RESOURCES = 

