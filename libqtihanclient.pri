
INCLUDEPATH += $$PWD/include/
DEPENDPATH += $$PWD
DEFINES += MUSCLE_SINGLE_THREAD_ONLY KITEMMODELS_STATIC_DEFINE

SOURCES += 	$$PWD/src/DeviceModel.cpp \
		$$PWD/src/MessageHandler.cpp \
		$$PWD/src/QtiHanClient.cpp \
		$$PWD/src/modeltest.cpp \
		$$PWD/src/kdescendantsproxymodel.cpp

HEADERS += 	$$PWD/include/QtiHanClient/DeviceModel.h \
		$$PWD/include/QtiHanClient/QtiHanClient.h \
		$$PWD/include/QtiHanClient/MessageHandler.h \
		$$PWD/include/QtiHanClient/modeltest.h \
		$$PWD/include/QtiHanClient/kdescendantsproxymodel.h \
		$$PWD/include/QtiHanClient/kbihash_p.h \
		$$PWD/include/QtiHanClient/kitemmodels_export.h \
		$$PWD/include/QtiHanClient/QtiHanClientDefs.h
