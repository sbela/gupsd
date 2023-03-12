QT -= gui
QT += network sql
CONFIG += c++2a console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        aslogger.cpp \
        db.cpp \
        main.cpp \
        upsdb.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    .clang-format \
    .gitignore

HEADERS += \
    asfx.h \
    aslogger.h \
    db.h \
    upsdb.h
