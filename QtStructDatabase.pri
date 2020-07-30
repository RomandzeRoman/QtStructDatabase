include($$PWD/QtTupleConversions/QtTupleConversions.pri)
include($$PWD/QtMultiThread/QtMultiThread.pri)
QT += sql
HEADERS += \
    $$PWD/private/asyncdatabase.h \
    $$PWD/private/databaserecord.h \
    $$PWD/private/databasetableviewmodel.h \
    $$PWD/private/databaseviewmodeldetail.h \
    $$PWD/private/databaseviewmodeldetail_vector.h \
    $$PWD/private/eventdatabasedetail.h \
    $$PWD/private/eventdatabaseprivate.h \
    $$PWD/private/taskedlistmodel.h \
    $$PWD/private/taskedobject.h \
    $$PWD/private/workerthread.h \
    $$PWD/database.h \
    $$PWD/database_detail.h \
    $$PWD/databaseviewmodel.h \
    $$PWD/eventdatabase.h \
    $$PWD/eventdatabaserecord.h \
    $$PWD/filter.h

SOURCES += \
    $$PWD/private/eventdatabaseprivate.cpp \
    $$PWD/private/taskedlistmodel.cpp \
    $$PWD/private/taskedobject.cpp \
    $$PWD/private/workerthread.cpp
OTHER_FILES += \
    $$PWD/CommonDatabase.dox
