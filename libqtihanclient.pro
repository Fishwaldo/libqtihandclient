#-------------------------------------------------
#
# Project created by QtCreator 2014-03-15T21:59:46
#
#-------------------------------------------------

QT       += core gui widgets

VERSION = 0.1.0

TARGET = qtihanclient
TEMPLATE = lib
CONFIG += lib_bundle c++11 shared link_pkgconfig create_pc create_prl no_install_prl silent

!isEmpty(IHANCLIENTPATH) {
	#message($$IHANCLIENTPATH/include/iHanClient/MsgTypes.hpp)
	exists($$IHANCLIENTPATH/include/iHanClient/MsgTypes.hpp) {
		message("Using Custom Path to iHanClient")
		INCLUDEPATH += $$IHANCLIENTPATH/include/ $$IHANCLIENTPATH/muscle/
		LIBS += -L$$IHANCLIENTPATH -lihanclient	
		CONFIG += staticlib
		CONFIG -= create_pc create_prl no_install_prl
	}
} else {
	PKGCONFIG += libihanclient
}


include (libqtihanclient.pri)

QMAKE_PKGCONFIG_NAME = libQTiHanClient
QMAKE_PKGCONFIG_DESCRIPTION = Qt bindings for the iHanClient Library
QMAKE_PKGCONFIG_PREFIX = $$INSTALLBASE
QMAKE_PKGCONFIG_LIBDIR = $$target.path
QMAKE_PKGCONFIG_INCDIR = $$headers.path
QMAKE_PKGCONFIG_VERSION = $$VERSION


