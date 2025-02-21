# Project Configuration
TEMPLATE = app
CONFIG += console c++23
QT += core gui widgets qml quick

# Target Executable Name
TARGET = download_package

# Source Files
SOURCES += main.cpp

# Header Files
HEADERS += taskbar.h
HEADERS += system_script.h
HEADERS += iso_creator_tools.h
HEADERS += arch_installer_script.h
HEADERS += archinstallergui.h
