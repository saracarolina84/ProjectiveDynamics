# Adding a graphical user interface around a <code>QGLViewer</code>.

# Qt's <code>designer</code> has been used to create a very simple interface example, described by a
# <code>.ui</code> user interface description file.

# Install the QGLViewer designer plugin so that the QGLViewer appears in the designer's widgets
# tabs. You can then connect signals and slots to and from the viewer. The viewer is fully
# functional and can be manipulated when you test your interface in <code>designer</code>.

# The plugin code is in the <code>designerPlugin</code> directory. Start from there to create
# plugins for the classes you will derive from QGLViewer. Select your architecture in the <a
# href="../download.html">download page</a> for details on the plugin compilation and installation.

# Here we use three slots and three signals (axis, grid and fps) to connect to and from the
# interface and the viewer.
#QMAKE_MAC_SDK = macosx10.12

TEMPLATE = app
TARGET   = projectiveDynamics

QMAKE_CC = clang-omp
QMAKE_CXX = clang++-omp
QMAKE_LINK = clang++-omp

QMAKE_CXXFLAGS += -fopenmp
QMAKE_LFLAGS += -fopenmp
LIBS += -L/usr/local/Cellar/llvm/5.0.0/lib/ -liomp5

HEADERS  = \
    viewer.h \
    springConstraint.h \
    simulator.h \
    utils.h \
    positionConstraint.h \
    tetraConstraint.h \
    springBody.h \
    projectiveBody.h \
    staticBody.h \
    projectiveConstraint.h \
    tetraBody.h \
    viewerInterface.h \
    hairBody.h
SOURCES  = main.cpp \
    viewer.cpp \
    springConstraint.cpp \
    simulator.cpp \
    utils.cpp \
    positionConstraint.cpp \
    tetraConstraint.cpp \
    springBody.cpp \
    tetraBody.cpp \
    viewerInterface.cpp \
    hairBody.cpp \
    projectiveConstraint.cpp

FORMS *= viewerInterface.ui

CONFIG += qt
QT *= xml opengl widgets gui

DEFINES += PROJECT_FOLDER=\\\"$$PWD/..\\\"



LIBS += -L$$PWD/../../libQGLViewer-2.7.0/QGLViewer/ -lQGLViewer

INCLUDEPATH += $$PWD/../../libQGLViewer-2.7.0/
DEPENDPATH += $$PWD/../../libQGLViewer-2.7.0/

PRE_TARGETDEPS += $$PWD/../../libQGLViewer-2.7.0/QGLViewer/libQGLViewer.a


## Linear Alebra
CONFIG(debug, debug|release) {
    LIBS += -L$$PWD/../../ElasticFEM/build-LinearAlgebra-Desktop-Debug/ -lLinearAlgebra
    DEPENDPATH += $$PWD/../../ElasticFEM/build-LinearAlgebra-Desktop-Debug
    PRE_TARGETDEPS += $$PWD/../../ElasticFEM/build-LinearAlgebra-Desktop-Debug/libLinearAlgebra.a
}
else {
    LIBS += -L$$PWD/../../ElasticFEM/build-LinearAlgebra-Desktop-Release/ -lLinearAlgebra
    DEPENDPATH += $$PWD/../../ElasticFEM/build-LinearAlgebra-Desktop-Release
    PRE_TARGETDEPS += $$PWD/../../ElasticFEM/build-LinearAlgebra-Desktop-Release/libLinearAlgebra.a
}
INCLUDEPATH += $$PWD/../../ElasticFEM/LinearAlgebra/inc

## TriMesh
CONFIG(debug, debug|release) {
    LIBS += -L$$PWD/../../ElasticFEM/build-TriMesh-Desktop-Debug/ -lTriMesh
    DEPENDPATH += $$PWD/../../ElasticFEM/build-TriMesh-Desktop-Debug
    PRE_TARGETDEPS += $$PWD/../../ElasticFEM/build-TriMesh-Desktop-Debug/libTriMesh.a
}
else {
    LIBS += -L$$PWD/../../ElasticFEM/build-TriMesh-Desktop-Release/ -lTriMesh
    DEPENDPATH += $$PWD/../../ElasticFEM/build-TriMesh-Desktop-Release
    PRE_TARGETDEPS += $$PWD/../../ElasticFEM/build-TriMesh-Desktop-Release/libTriMesh.a
}
INCLUDEPATH += $$PWD/../../ElasticFEM/TriMesh/inc


## CDQueries
CONFIG(debug, debug|release) {
    LIBS += -L$$PWD/../../ElasticFEM/build-CDQueries-Desktop-Debug/ -lCDQueries
    DEPENDPATH += $$PWD/../../ElasticFEM/build-CDQueries-Desktop-Debug
    PRE_TARGETDEPS += $$PWD/../../ElasticFEM/build-CDQueries-Desktop-Debug/libCDQueries.a
}
else {
    LIBS += -L$$PWD/../../ElasticFEM/build-CDQueries-Desktop-Release/ -lCDQueries
    DEPENDPATH += $$PWD/../../ElasticFEM/build-CDQueries-Desktop-Release
    PRE_TARGETDEPS += $$PWD/../../ElasticFEM/build-CDQueries-Desktop-Release/libCDQueries.a
}
INCLUDEPATH += $$PWD/../../ElasticFEM/CDQueries/inc


### RigidBody
#CONFIG(debug, debug|release) {
#    LIBS += -L$$PWD/../../ElasticFEM/build-RigidBody-Desktop-Debug/ -lRigidBody
#    DEPENDPATH += $$PWD/../../ElasticFEM/build-RigidBody-Desktop-Debug
#    PRE_TARGETDEPS += $$PWD/../../ElasticFEM/build-RigidBody-Desktop-Debug/libRigidBody.a
#}
#else {
#    LIBS += -L$$PWD/../../ElasticFEM/build-RigidBody-Desktop-Release/ -lRigidBody
#    DEPENDPATH += $$PWD/../../ElasticFEM/build-RigidBody-Desktop-Release
#    PRE_TARGETDEPS += $$PWD/../../ElasticFEM/build-RigidBody-Desktop-Release/libRigidBody.a
#}
#INCLUDEPATH += $$PWD/../../ElasticFEM/RigidBody/inc

## Contact
CONFIG(debug, debug|release) {
    LIBS += -L$$PWD/../../ElasticFEM/build-Contact-Desktop-Debug/ -lContact
    DEPENDPATH += $$PWD/../../ElasticFEM/build-Contact-Desktop-Debug
    PRE_TARGETDEPS += $$PWD/../../ElasticFEM/build-Contact-Desktop-Debug/libContact.a
}
else {
    LIBS += -L$$PWD/../../ElasticFEM/build-Contact-Desktop-Release/ -lContact
    DEPENDPATH += $$PWD/../../ElasticFEM/build-Contact-Desktop-Release
    PRE_TARGETDEPS += $$PWD/../../ElasticFEM/build-Contact-Desktop-Release/libContact.a
}
INCLUDEPATH += $$PWD/../../ElasticFEM/Contact/inc


## FEM
CONFIG(debug, debug|release) {
    LIBS += -L$$PWD/../../ElasticFEM/build-FEM-Desktop-Debug/ -lFEM
    DEPENDPATH += $$PWD/../../ElasticFEM/build-FEM-Desktop-Debug
    PRE_TARGETDEPS += $$PWD/../../ElasticFEM/build-FEM-Desktop-Debug/libFEM.a
}
else {
    LIBS += -L$$PWD/../../ElasticFEM/build-FEM-Desktop-Release/ -lFEM
    DEPENDPATH += $$PWD/../../ElasticFEM/build-FEM-Desktop-Release
    PRE_TARGETDEPS += $$PWD/../../ElasticFEM/build-FEM-Desktop-Release/libFEM.a
}
INCLUDEPATH += $$PWD/../../ElasticFEM/FEM/inc

INCLUDEPATH += $$PWD/../../eigen


INCLUDEPATH += /usr/local/Cellar/opencv/3.3.0_2/include/
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.10
LIBS += -L/usr/local/Cellar/opencv/3.3.0_2/lib -lopencv_imgcodecs -lopencv_highgui -lopencv_video -lopencv_optflow -lopencv_tracking -lopencv_videoio -lopencv_core -lopencv_plot -lopencv_imgproc -lopencv_objdetect
