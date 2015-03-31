 HEADERS       = mainwindow.h ../common/utils.h ../common/pid.h ../common/common.h applications.h \
		  pid_thread.h db.h dbstorage.h dbpid.h stats_dialog.h graph.h
 SOURCES       = mainwindow.cpp \
                 main.cpp \
                 stats_dialog.cpp \
                 pid_thread.cpp \
                 db.cpp \
                 dbpid.cpp \
                 graph.cpp \
                  ../common/utils.cpp \
                  ../common/pid.cpp \
                  applications.cpp
RESOURCES = firetools.qrc
TARGET=../../build/firetools
