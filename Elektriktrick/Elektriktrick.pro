#-------------------------------------------------
#
# Project created by QtCreator 2014-06-26T14:35:20
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Elektriktrick
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    ETTriangle.cpp \
    ETVector.cpp \
    glwidget.cpp \
    ETModel.cpp \
    ETVertex.cpp \
    ETEdge.cpp \
    ETImport.cpp \
    ETImportSTL.cpp \
    ETExport.cpp \
    ETExportSTL.cpp \
    ETApp.cpp

HEADERS  += mainwindow.h \
    ETTriangle.h \
    ETVector.h \
    glwidget.h \
    ETModel.h \
    ETVertex.h \
    ETEdge.h \
    ETImport.h \
    ETImportSTL.h \
    ETExport.h \
    ETExportSTL.h \
    ETApp.h

FORMS    += mainwindow.ui

OTHER_FILES += \
    Notes.txt
