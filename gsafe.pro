#-------------------------------------------------
#
# gSAFE shared library project file
#
#-------------------------------------------------

TEMPLATE	= lib
QT 		+= gui network sql xml widgets printsupport 
CONFIG  	+= qt warn_on
DEFINES 	+= QT_DLL FORMS_HAS_UI_PREFIX

HEADERS 	+= \
                datalib.h    \
                dconsole.h   \
                dialib.h     \
                docgen.h     \
                guilib.h     \
                hfactory.h   \
                printlib.h   \
                res_p.h      \
                xmlolib.h    \
                xbio.h	     \
                gstexts.h    \
                xbioconn.h
                        
SOURCES 	+= \
                datalib.cpp  \
                dconsole.cpp \
                dialib.cpp   \
                docgen.cpp   \
                guilib.cpp   \
                hfactory.cpp \
                printlib.cpp \
                xmlolib.cpp  \
                xbio.cpp     \
                xbioconn.cpp

RESOURCES += gsafe.qrc
INCLUDEPATH += .

#end
