#-------------------------------------------------
#
# Project created by QtCreator 2016-11-03T19:43:57
#
#-------------------------------------------------

QT += core gui multimedia svg

CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = radio-ui
TEMPLATE = app

SOURCES *= \
	main.cpp \
	RadioGui.cpp \
	LogoDownloader.cpp

HEADERS *= \
	RadioGui.h \
	LogoDownloader.h

FORMS *= \
	RadioGui.ui

RESOURCES *= \
	RadioGui.qrc
