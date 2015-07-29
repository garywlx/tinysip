TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c

unix {
        DESTDIR = bin/linux
        LIBDIR = ../lib/linux
        BUILDDIR = ../build/linux
        PJSIP_DIR = /home/brian/Projects/SIP/dev/pjproject-2.4
        PJSIP_TARGET = x86_64-unknown-linux-gnu #change this to os-depend target
}

INCLUDEPATH += $$SOURCEDIR/GeneratedFiles \
    $$SOURCEDIR/GeneratedFiles/Debug \
    $$SOURCEDIR \
    $$VENDORDIR/qt-json \
    $$PJSIP_DIR \
        $$PJSIP_DIR/pjmedia/include \
    $$PJSIP_DIR/pjsip/include \
    $$PJSIP_DIR/pjnath/include \
    $$PJSIP_DIR/pjmedia/include/pjmedia-codec \
    $$PJSIP_DIR/pjmedia/include/pjmedia-audiodev \
    $$PJSIP_DIR/pjmedia/include/pjmedia \
    $$PJSIP_DIR/pjlib-util/include \
    $$PJSIP_DIR/pjlib/include
unix: INCLUDEPATH += /usr/include/

LIBS += -L/usr/lib/ \
        -L$$LIBDIR/ \
        -L$$PJSIP_DIR/third_party/lib \
        -L$$PJSIP_DIR/pjsip/lib \
        -L$$PJSIP_DIR/pjnath/lib \
        -L$$PJSIP_DIR/pjmedia/lib \
        -L$$PJSIP_DIR/pjlib-util/lib \
        -L$$PJSIP_DIR/pjlib/lib \

unix: LIBS += -L/usr/lib/ \
-lpjsua2-$$PJSIP_TARGET -lstdc++ -lpjsua-$$PJSIP_TARGET \
-lpjsip-ua-$$PJSIP_TARGET -lpjsip-simple-$$PJSIP_TARGET -lpjsip-$$PJSIP_TARGET \
-lpjmedia-codec-$$PJSIP_TARGET -lpjmedia-$$PJSIP_TARGET -lpjmedia-videodev-$$PJSIP_TARGET \
-lpjmedia-audiodev-$$PJSIP_TARGET -lpjmedia-$$PJSIP_TARGET -lpjnath-$$PJSIP_TARGET \
-lpjlib-util-$$PJSIP_TARGET  -lsrtp-$$PJSIP_TARGET -lresample-$$PJSIP_TARGET -lgsmcodec-$$PJSIP_TARGET \
-lspeex-$$PJSIP_TARGET -lilbccodec-$$PJSIP_TARGET -lg7221codec-$$PJSIP_TARGET -lportaudio-$$PJSIP_TARGET \
-lpj-$$PJSIP_TARGET -luuid -lm -lrt -lpthread -lasound


include(deployment.pri)
qtcAddDeployment()
