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

# DEFINES += MODULE_REFRESHAGENT_NETSERVER
# DEFINES += MODULE_REFRESHAGENT_NETCLIENT

INCLUDEPATH += $${GSAFE_PATH}

SOURCES += \
    $${GSAFE_PATH}data.cpp \
    $${GSAFE_PATH}dm.cpp \
    $${GSAFE_PATH}dmext.cpp \
    $${GSAFE_PATH}dmjson.cpp \
    $${GSAFE_PATH}builder.cpp \
    $${GSAFE_PATH}ra.cpp \
    $${GSAFE_PATH}gui.cpp \
    $${GSAFE_PATH}guiext.cpp \
    $${GSAFE_PATH}quicktable.cpp \
    $${GSAFE_PATH}ftypes.cpp \
    $${GSAFE_PATH}database.cpp \
    $${GSAFE_PATH}dialog.cpp \
    $${GSAFE_PATH}xo.cpp \
    $${GSAFE_PATH}po.cpp \
    $${GSAFE_PATH}dconsole.cpp \
    $${GSAFE_PATH}xbio.cpp \
    $${GSAFE_PATH}xbioconn.cpp \
    $${GSAFE_PATH}template_inits.cpp

HEADERS  += \
    $${GSAFE_PATH}/gSafe \
    $${GSAFE_PATH}data.h \
    $${GSAFE_PATH}dm.h \
    $${GSAFE_PATH}dmext.h \
    $${GSAFE_PATH}builder.h \
    $${GSAFE_PATH}ra.h \
    $${GSAFE_PATH}gui.h \
    $${GSAFE_PATH}guiext.h \
    $${GSAFE_PATH}quicktable.h \
    $${GSAFE_PATH}ftypes.h \
    $${GSAFE_PATH}database.h \
    $${GSAFE_PATH}dialog.h \
    $${GSAFE_PATH}xo.h \
    $${GSAFE_PATH}po.h \
    $${GSAFE_PATH}dconsole.h \
    $${GSAFE_PATH}xbio.h \
    $${GSAFE_PATH}xbioconn.h \
    $${GSAFE_PATH}tmpl_hfwvs.tpp \
    $${GSAFE_PATH}tmpl_hkvs.tpp

RESOURCES += $${GSAFE_PATH}gsafe.qrc

# end of gsafe.pri
