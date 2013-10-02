#-------------------------------------------------
#
# gSAFE
#
#-------------------------------------------------

TEMPLATE	=	lib
QT 			  +=  gui widgets network sql xml webkitwidgets printsupport
CONFIG  	+=	qt warn_on 
DEFINES 	+=	QT_DLL FORMS_HAS_UI_PREFIX

HEADERS 	+= 	datalib.h    \
                dconsole.h   \
                dialib.h     \
                docgen.h     \
                guilib.h     \
                hfactory.h   \
                printlib.h   \
                res_p.h      \
                xmlolib.h    \
                xbio.h		 \
                xbioconn.h
                        
SOURCES 	+= 	datalib.cpp  \
                dconsole.cpp \
                dialib.cpp   \
                docgen.cpp   \
                guilib.cpp   \
                hfactory.cpp \
                printlib.cpp \
                xmlolib.cpp  \
                xbio.cpp     \
                xbioconn.cpp

FORMS   	+= 	debugwidgetbase.ui

INCLUDEPATH	+= 	.

# QT += webkit

# INCLUDEPATH	+= 	../XBaseLib/
# DEFINES		+= ENABLE_XBASE_SUPPORT

#end
