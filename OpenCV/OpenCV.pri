INCLUDEPATH += $$PWD/include \

android {
    LIBS += $$PWD/lib/armeabi-v7a/libopencv_core.a \
            $$PWD/lib/armeabi-v7a/libopencv_objdetect.a \
            $$PWD/lib/armeabi-v7a/libopencv_highgui.a \
            $$PWD/lib/armeabi-v7a/libopencv_imgproc.a \
            $$PWD/lib/armeabi-v7a/libopencv_java.so \

    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android-sources
}

win32 {
    contains(QMAKE_HOST.arch, x86_64) {
        CONFIG(debug, debug|release) {
            LIBS += -L$$PWD/lib/x64/vc11/debug \
                    -lopencv_core2410d \
                    -lopencv_objdetect2410d  \
                    -lopencv_highgui2410d \
                    -lopencv_imgproc2410d \
        } else {
            LIBS += -L$$PWD/lib/x64/vc11/release \
                    -lopencv_core2410 \
                    -lopencv_objdetect2410  \
                    -lopencv_highgui2410 \
                    -lopencv_imgproc2410 \
        }
    } else {
        CONFIG(debug, debug|release) {
            LIBS += -L$$PWD/lib/x86/vc11/debug \
                    -lopencv_core2410d \
                    -lopencv_objdetect2410d  \
                    -lopencv_highgui2410d \
                    -lopencv_imgproc2410d \
        } else {
            LIBS += -L$$PWD/lib/x86/vc11/release \
                    -lopencv_core2410 \
                    -lopencv_objdetect2410  \
                    -lopencv_highgui2410 \
                    -lopencv_imgproc2410 \
        }
    }
}
