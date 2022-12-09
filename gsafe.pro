#-------------------------------------------------
#
# gSAFE shared library project file
#
#-------------------------------------------------

TEMPLATE	= lib
QT 		+= gui network sql xml widgets printsupport 
CONFIG  	+= qt warn_on
DEFINES 	+= QT_DLL FORMS_HAS_UI_PREFIX

SOURCES += \
    data.cpp \
    dm.cpp \
    dmext.cpp \
    dmjson.cpp \
    builder.cpp \
    ra.cpp \
    gui.cpp \
    guiext.cpp \
    quicktable.cpp \
    ftypes.cpp \
    database.cpp \
    dialog.cpp \
    xo.cpp \
    po.cpp \
    dconsole.cpp \
    xbio.cpp \
    xbioconn.cpp

HEADERS  += \
    data.h \
    dm.h \
    dmext.h \
    builder.h \
    ra.h \
    gui.h \
    guiext.h \
    quicktable.h \
    ftypes.h \
    database.h \
    dialog.h \
    xo.h \
    po.h \
    dconsole.h \
    xbio.h \
    xbioconn.h

RESOURCES += gsafe.qrc
INCLUDEPATH += .

#end
