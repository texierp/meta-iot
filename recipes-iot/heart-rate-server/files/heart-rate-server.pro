TEMPLATE = app
TARGET = heartrate-server

QT += bluetooth
CONFIG += c++11

SOURCES += main.cpp

target.path = /usr/bin
INSTALLS += target
