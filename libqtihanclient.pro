#-------------------------------------------------
#
# Project created by QtCreator 2014-03-15T21:59:46
#
#-------------------------------------------------

QT       += core gui widgets network

include (qtihanclientconfig.pri)



TARGET = qt$${QT_MAJOR_VERSION}ihanclient
TEMPLATE = lib
CONFIG += lib_bundle c++11 shared  silent debug_and_release

unix { 
	CONFIG += link_pkgconfig create_pc create_prl no_install_prl
}
win32 {
	DEFINES += NOMINMAX QTIHANCLIENT_MAKEDLL
}

unix {
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
}
win32 {
	exists(ihanclient/include/iHanClient/MsgTypes.hpp) {
		message("Using iHanClient Export at ihanclient")
		INCLUDEPATH += ihanclient/include/ ihanclient/include/muscle/
		CONFIG(debug, debug|release) {
			LIBS += -Lihanclient/Debug/libs/ -lihanclient
		}
		CONFIG(release, debug|release) {
			LIBS += -Lihanclient/Release/libs/ -lihanclient
		}
		message("Using Libs: $${LIBS}")
	} else {
		error("ihanclient headers and libs not at $$PWD/ihanclient")
	}
}




include (libqtihanclient.pri)
unix {
	target.path = $${QIC_INSTALL_LIBS}
	headers.files = $${HEADERS}
	headers.path = $${QIC_INSTALL_HEADERS}

	docs.path = $${QIC_INSTALL_DOCS}
	docs.files = README AUTHORS ChangeLog COPYING NEWS
	pkgconfig.path = $${QIC_INSTALL_PC}

	QMAKE_PKGCONFIG_NAME = libQTiHanClient
	QMAKE_PKGCONFIG_DESCRIPTION = Qt bindings for the iHanClient Library
	QMAKE_PKGCONFIG_PREFIX = $$INSTALLBASE
	QMAKE_PKGCONFIG_LIBDIR = $$target.path
	QMAKE_PKGCONFIG_INCDIR = $$QIC_HEADERS_PATH
	QMAKE_PKGCONFIG_VERSION = $$VERSION
	QMAKE_PKGCONFIG_DESTDIR = pkgconfig

	INSTALLS += target headers docs

}
