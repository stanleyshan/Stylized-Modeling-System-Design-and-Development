INCLUDEPATH += $$PWD/src/

DEFINES += _USE_MATH_DEFINES

CONFIG(debug, debug|release) {

    android {
        LIBS += $$PWD\build-OpenMesh-Android_for_armeabi_v7a_GCC_4_9_Qt_5_4_0-Debug\libOpenMesh.so
        ANDROID_EXTRA_LIBS += $$PWD\build-OpenMesh-Android_for_armeabi_v7a_GCC_4_9_Qt_5_4_0-Debug\libOpenMesh.so
        message("debug:android")
    }

    ios {
        LIBS += $$PWD/Openmesh_iOSlib/Debug-iphoneos/libOpenmesh.a
        LIBS += $$PWD/Openmesh_iOSlib/Debug-iphonesimulator/libOpenmesh.a
        message("debug:ios")
    }

    win32 {
        contains(QMAKE_TARGET.arch, x86_64) {
            LIBS += $$PWD\build-OpenMesh-Desktop_Qt_5_4_0_MSVC2013_64bit-Debug\OpenMesh.lib
            message("debug:x86_64")
        }
    }

} else {

    android {
        LIBS += $$PWD\build-OpenMesh-Android_for_armeabi_v7a_GCC_4_9_Qt_5_4_0-Release\libOpenMesh.so
        ANDROID_EXTRA_LIBS += $$PWD\build-OpenMesh-Android_for_armeabi_v7a_GCC_4_9_Qt_5_4_0-Release\libOpenMesh.so
        message("release:android")
    }

    ios {
        LIBS += $$PWD/Openmesh_iOSlib/Release-iphoneos/libOpenmesh.a
        LIBS += $$PWD/Openmesh_iOSlib/Release-iphonesimulator/libOpenmesh.a
        message("release:ios")
    }

    win32 {
        contains(QMAKE_TARGET.arch, x86_64) {
            LIBS += $$PWD\build-OpenMesh-Desktop_Qt_5_4_0_MSVC2013_64bit-Release\OpenMesh.lib
            message("release:x86_64")
        }
    }

}
