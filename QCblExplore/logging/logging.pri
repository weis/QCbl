INCLUDEPATH += $$PWD
#DEFINES += QS_LOG_LINE_NUMBERS    # automatically writes the file and line for each log message
#DEFINES += QS_LOG_DISABLE         # logging code is replaced with a no-op

DEFINES += QS_LOG_SEPARATE_THREAD  # messages are queued and written from a separate thread
SOURCES += $$PWD/QsLog/QsLogDest.cpp \
    $$PWD/QsLog/QsLog.cpp \
    $$PWD/QsLog/QsLogDestConsole.cpp \
    $$PWD/QsLog/QsLogDestFile.cpp \
    $$PWD/QsLog/QsLogDestFunctor.cpp \
    $$PWD/loginternal.cpp

HEADERS += $$PWD/QsLog/QsLogDest.h \
    $$PWD/QsLog/QsLog.h \
    $$PWD/QsLog/QsLogDestConsole.h \
    $$PWD/QsLog/QsLogLevel.h \
    $$PWD/QsLog/QsLogDestFile.h \
    $$PWD/QsLog/QsLogDisableForThisFile.h \
    $$PWD/QsLog/QsLogDestFunctor.h \
    $$PWD/loginternal.h

OTHER_FILES += \
    $$PWD/QsLog/QsLogChanges.txt \
    $$PWD/QsLog/QsLogReadme.txt \
    $$PWD/QsLog/LICENSE.txt
