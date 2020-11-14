#-------------------------------------------------
#
# gSAFE 
#      http://hyperprog.com/gsafe/
#
#-------------------------------------------------

isEmpty(GSAFE_PATH) {
    message( 'GSAFE_PATH is empty. use default value.' )
    GSAFE_PATH = $$PWD/../gSAFE/
} else {
    message( 'GSAFE_PATH is not empty.' )
    message( $${GSAFE_PATH} )
}

INCLUDEPATH += $${GSAFE_PATH}

SOURCES += \
    $${GSAFE_PATH}xmlolib.cpp \
    $${GSAFE_PATH}xbio.cpp \
    $${GSAFE_PATH}xbioconn.cpp \
    $${GSAFE_PATH}printlib.cpp \
    $${GSAFE_PATH}hfactory.cpp \
    $${GSAFE_PATH}guilib.cpp \
    $${GSAFE_PATH}docgen.cpp \
    $${GSAFE_PATH}dialib.cpp \
    $${GSAFE_PATH}dconsole.cpp \
    $${GSAFE_PATH}datalib.cpp 

HEADERS  += \
    $${GSAFE_PATH}xbio.h \
    $${GSAFE_PATH}xbioconn.h \
    $${GSAFE_PATH}res_p.h \
    $${GSAFE_PATH}printlib.h \
    $${GSAFE_PATH}hfactory.h \
    $${GSAFE_PATH}guilib.h \
    $${GSAFE_PATH}gstexts.h \
    $${GSAFE_PATH}docgen.h \
    $${GSAFE_PATH}dialib.h \
    $${GSAFE_PATH}dconsole.h \
    $${GSAFE_PATH}datalib.h \
    $${GSAFE_PATH}xmlolib.h

RESOURCES += $${GSAFE_PATH}gsafe.qrc

# end of gsafe.pri
