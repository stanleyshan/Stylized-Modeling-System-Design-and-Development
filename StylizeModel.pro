TARGET = StylizeModel
TEMPLATE = app

QT += qml quick\
      gui-private \
      multimedia \
      gui \

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += $$PWD/include/eigen \
               $$PWD/include \
               $$PWD/include/dlib-18.18 \

LIBS += $$PWD/lib/libdlib.a


include(OpenMesh3.3/OpenMesh.pri)
include(OpenCV/OpenCV.pri)

SOURCES +=  \
    main.cpp \
    meshrenderer.cpp \
    qopenmeshobject.cpp \
    expmap.cpp \
    mainwindow.cpp \
    myglwidget.cpp \
    mylistview.cpp \
    HSplinePath.cpp \
    strokedeform.cpp \
    facetex.cpp \
    plugin.cpp \
    Clm.cpp \
    FFT.cpp \
    MosseFilter.cpp \
    SvmFilter.cpp \
    PoissonBlender.cpp

RESOURCES += \
    qrcFiles/image.qrc \
    qrcFiles/model.qrc \
    qrcFiles/qml.qrc \
    qrcFiles/shader.qrc \
    qrcFiles/texture.qrc \
    qrcFiles/json.qrc \
    qrcFiles/xml.qrc \
    qrcFiles/svm.qrc

HEADERS += \
    meshrenderer.h \
    qopenmeshobject.h \
    expmap.h \
    mainwindow.h \
    myglwidget.h \
    mylistview.h \
    HSplineCore.h \
    HSplinePath.h \
    SplineSample.h \
    strokedeform.h \
    mymesh.h \
    facetex.h \
    plugin.h \
    Clm.h \
    FFT.h \
    MosseFilter.h \
    numeric.h \
    Params.h \
    SvmFilter.h \
    PoissonBlender.h

FORMS += \
    mainwindow.ui
