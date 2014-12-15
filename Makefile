#############################################################################
# Makefile for building: peerster
# Generated by qmake (2.01a) (Qt 4.8.5) on: Sun Dec 14 18:50:48 2014
# Project:  peerster.pro
# Template: app
# Command: /usr/bin/qmake-qt4 -o Makefile peerster.pro
#############################################################################

####### Compiler, tools and options

CC            = gcc
CXX           = g++
DEFINES       = -DQT_NO_DEBUG -DQT_GUI_LIB -DQT_NETWORK_LIB -DQT_CORE_LIB -DQT_SHARED
CFLAGS        = -pipe -O2 -g -pipe -Wall -Wp,-D_FORTIFY_SOURCE=2 -fstack-protector-strong --param=ssp-buffer-size=4 -grecord-gcc-switches -m64 -mtune=generic -O2 -Wall -W -D_REENTRANT $(DEFINES)
CXXFLAGS      = -pipe -O2 -g -pipe -Wall -Wp,-D_FORTIFY_SOURCE=2 -fstack-protector-strong --param=ssp-buffer-size=4 -grecord-gcc-switches -m64 -mtune=generic -O2 -Wall -W -D_REENTRANT $(DEFINES)
INCPATH       = -I/usr/lib64/qt4/mkspecs/linux-g++ -I. -I/usr/include/QtCore -I/usr/include/QtNetwork -I/usr/include/QtGui -I/usr/include -I. -I/usr/include/QtCrypto -I.
LINK          = g++
LFLAGS        = -Wl,-O1 -Wl,-z,relro
LIBS          = $(SUBLIBS)  -L/usr/lib64 -L/usr/lib64 -lqca -lQtGui -lQtNetwork -lQtCore -lpthread 
AR            = ar cqs
RANLIB        = 
QMAKE         = /usr/bin/qmake-qt4
TAR           = tar -cf
COMPRESS      = gzip -9f
COPY          = cp -f
SED           = sed
COPY_FILE     = $(COPY)
COPY_DIR      = $(COPY) -r
STRIP         = 
INSTALL_FILE  = install -m 644 -p
INSTALL_DIR   = $(COPY_DIR)
INSTALL_PROGRAM = install -m 755 -p
DEL_FILE      = rm -f
SYMLINK       = ln -f -s
DEL_DIR       = rmdir
MOVE          = mv -f
CHK_DIR_EXISTS= test -d
MKDIR         = mkdir -p

####### Output directory

OBJECTS_DIR   = ./

####### Files

SOURCES       = main.cc \
		netsocket.cc \
		peermanager.cc \
		peer.cc \
		filemanager.cc \
		fileconnection.cc \
		shamir.cc \
		secretmanager.cc \
		dht.cc moc_main.cpp \
		moc_netsocket.cpp \
		moc_peermanager.cpp \
		moc_peer.cpp \
		moc_filemanager.cpp \
		moc_fileconnection.cpp \
		moc_shamir.cpp \
		moc_secretmanager.cpp \
		moc_dht.cpp
OBJECTS       = main.o \
		netsocket.o \
		peermanager.o \
		peer.o \
		filemanager.o \
		fileconnection.o \
		shamir.o \
		secretmanager.o \
		dht.o \
		moc_main.o \
		moc_netsocket.o \
		moc_peermanager.o \
		moc_peer.o \
		moc_filemanager.o \
		moc_fileconnection.o \
		moc_shamir.o \
		moc_secretmanager.o \
		moc_dht.o
DIST          = /usr/lib64/qt4/mkspecs/common/unix.conf \
		/usr/lib64/qt4/mkspecs/common/linux.conf \
		/usr/lib64/qt4/mkspecs/common/gcc-base.conf \
		/usr/lib64/qt4/mkspecs/common/gcc-base-unix.conf \
		/usr/lib64/qt4/mkspecs/common/g++-base.conf \
		/usr/lib64/qt4/mkspecs/common/g++-unix.conf \
		/usr/lib64/qt4/mkspecs/qconfig.pri \
		/usr/lib64/qt4/mkspecs/features/qt_functions.prf \
		/usr/lib64/qt4/mkspecs/features/qt_config.prf \
		/usr/lib64/qt4/mkspecs/features/exclusive_builds.prf \
		/usr/lib64/qt4/mkspecs/features/default_pre.prf \
		/usr/lib64/qt4/mkspecs/features/release.prf \
		/usr/lib64/qt4/mkspecs/features/default_post.prf \
		/usr/lib64/qt4/mkspecs/features/crypto.prf \
		/usr/lib64/qt4/mkspecs/features/shared.prf \
		/usr/lib64/qt4/mkspecs/features/unix/gdb_dwarf_index.prf \
		/usr/lib64/qt4/mkspecs/features/warn_on.prf \
		/usr/lib64/qt4/mkspecs/features/qt.prf \
		/usr/lib64/qt4/mkspecs/features/unix/thread.prf \
		/usr/lib64/qt4/mkspecs/features/moc.prf \
		/usr/lib64/qt4/mkspecs/features/resources.prf \
		/usr/lib64/qt4/mkspecs/features/uic.prf \
		/usr/lib64/qt4/mkspecs/features/yacc.prf \
		/usr/lib64/qt4/mkspecs/features/lex.prf \
		/usr/lib64/qt4/mkspecs/features/include_source_dir.prf \
		peerster.pro
QMAKE_TARGET  = peerster
DESTDIR       = 
TARGET        = peerster

first: all
####### Implicit rules

.SUFFIXES: .o .c .cpp .cc .cxx .C

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cxx.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.C.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.c.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o "$@" "$<"

####### Build rules

all: Makefile $(TARGET)

$(TARGET):  $(OBJECTS)  
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(OBJCOMP) $(LIBS)

Makefile: peerster.pro  /usr/lib64/qt4/mkspecs/linux-g++/qmake.conf /usr/lib64/qt4/mkspecs/common/unix.conf \
		/usr/lib64/qt4/mkspecs/common/linux.conf \
		/usr/lib64/qt4/mkspecs/common/gcc-base.conf \
		/usr/lib64/qt4/mkspecs/common/gcc-base-unix.conf \
		/usr/lib64/qt4/mkspecs/common/g++-base.conf \
		/usr/lib64/qt4/mkspecs/common/g++-unix.conf \
		/usr/lib64/qt4/mkspecs/qconfig.pri \
		/usr/lib64/qt4/mkspecs/features/qt_functions.prf \
		/usr/lib64/qt4/mkspecs/features/qt_config.prf \
		/usr/lib64/qt4/mkspecs/features/exclusive_builds.prf \
		/usr/lib64/qt4/mkspecs/features/default_pre.prf \
		/usr/lib64/qt4/mkspecs/features/release.prf \
		/usr/lib64/qt4/mkspecs/features/default_post.prf \
		/usr/lib64/qt4/mkspecs/features/crypto.prf \
		/usr/lib64/qt4/mkspecs/features/shared.prf \
		/usr/lib64/qt4/mkspecs/features/unix/gdb_dwarf_index.prf \
		/usr/lib64/qt4/mkspecs/features/warn_on.prf \
		/usr/lib64/qt4/mkspecs/features/qt.prf \
		/usr/lib64/qt4/mkspecs/features/unix/thread.prf \
		/usr/lib64/qt4/mkspecs/features/moc.prf \
		/usr/lib64/qt4/mkspecs/features/resources.prf \
		/usr/lib64/qt4/mkspecs/features/uic.prf \
		/usr/lib64/qt4/mkspecs/features/yacc.prf \
		/usr/lib64/qt4/mkspecs/features/lex.prf \
		/usr/lib64/qt4/mkspecs/features/include_source_dir.prf \
		/usr/lib64/libqca.prl \
		/usr/lib64/libQtGui.prl \
		/usr/lib64/libQtCore.prl \
		/usr/lib64/libQtNetwork.prl
	$(QMAKE) -o Makefile peerster.pro
/usr/lib64/qt4/mkspecs/common/unix.conf:
/usr/lib64/qt4/mkspecs/common/linux.conf:
/usr/lib64/qt4/mkspecs/common/gcc-base.conf:
/usr/lib64/qt4/mkspecs/common/gcc-base-unix.conf:
/usr/lib64/qt4/mkspecs/common/g++-base.conf:
/usr/lib64/qt4/mkspecs/common/g++-unix.conf:
/usr/lib64/qt4/mkspecs/qconfig.pri:
/usr/lib64/qt4/mkspecs/features/qt_functions.prf:
/usr/lib64/qt4/mkspecs/features/qt_config.prf:
/usr/lib64/qt4/mkspecs/features/exclusive_builds.prf:
/usr/lib64/qt4/mkspecs/features/default_pre.prf:
/usr/lib64/qt4/mkspecs/features/release.prf:
/usr/lib64/qt4/mkspecs/features/default_post.prf:
/usr/lib64/qt4/mkspecs/features/crypto.prf:
/usr/lib64/qt4/mkspecs/features/shared.prf:
/usr/lib64/qt4/mkspecs/features/unix/gdb_dwarf_index.prf:
/usr/lib64/qt4/mkspecs/features/warn_on.prf:
/usr/lib64/qt4/mkspecs/features/qt.prf:
/usr/lib64/qt4/mkspecs/features/unix/thread.prf:
/usr/lib64/qt4/mkspecs/features/moc.prf:
/usr/lib64/qt4/mkspecs/features/resources.prf:
/usr/lib64/qt4/mkspecs/features/uic.prf:
/usr/lib64/qt4/mkspecs/features/yacc.prf:
/usr/lib64/qt4/mkspecs/features/lex.prf:
/usr/lib64/qt4/mkspecs/features/include_source_dir.prf:
/usr/lib64/libqca.prl:
/usr/lib64/libQtGui.prl:
/usr/lib64/libQtCore.prl:
/usr/lib64/libQtNetwork.prl:
qmake:  FORCE
	@$(QMAKE) -o Makefile peerster.pro

dist: 
	@$(CHK_DIR_EXISTS) .tmp/peerster1.0.0 || $(MKDIR) .tmp/peerster1.0.0 
	$(COPY_FILE) --parents $(SOURCES) $(DIST) .tmp/peerster1.0.0/ && $(COPY_FILE) --parents main.hh netsocket.hh peermanager.hh peer.hh filemanager.hh fileconnection.hh shamir.hh secretmanager.hh dht.hh .tmp/peerster1.0.0/ && $(COPY_FILE) --parents main.cc netsocket.cc peermanager.cc peer.cc filemanager.cc fileconnection.cc shamir.cc secretmanager.cc dht.cc .tmp/peerster1.0.0/ && (cd `dirname .tmp/peerster1.0.0` && $(TAR) peerster1.0.0.tar peerster1.0.0 && $(COMPRESS) peerster1.0.0.tar) && $(MOVE) `dirname .tmp/peerster1.0.0`/peerster1.0.0.tar.gz . && $(DEL_FILE) -r .tmp/peerster1.0.0


clean:compiler_clean 
	-$(DEL_FILE) $(OBJECTS)
	-$(DEL_FILE) *~ core *.core


####### Sub-libraries

distclean: clean
	-$(DEL_FILE) $(TARGET) 
	-$(DEL_FILE) Makefile


check: first

mocclean: compiler_moc_header_clean compiler_moc_source_clean

mocables: compiler_moc_header_make_all compiler_moc_source_make_all

compiler_moc_header_make_all: moc_main.cpp moc_netsocket.cpp moc_peermanager.cpp moc_peer.cpp moc_filemanager.cpp moc_fileconnection.cpp moc_shamir.cpp moc_secretmanager.cpp moc_dht.cpp
compiler_moc_header_clean:
	-$(DEL_FILE) moc_main.cpp moc_netsocket.cpp moc_peermanager.cpp moc_peer.cpp moc_filemanager.cpp moc_fileconnection.cpp moc_shamir.cpp moc_secretmanager.cpp moc_dht.cpp
moc_main.cpp: main.hh
	/usr/lib64/qt4/bin/moc $(DEFINES) $(INCPATH) main.hh -o moc_main.cpp

moc_netsocket.cpp: peer.hh \
		peermanager.hh \
		filemanager.hh \
		fileconnection.hh \
		secretmanager.hh \
		dht.hh \
		netsocket.hh
	/usr/lib64/qt4/bin/moc $(DEFINES) $(INCPATH) netsocket.hh -o moc_netsocket.cpp

moc_peermanager.cpp: peer.hh \
		peermanager.hh
	/usr/lib64/qt4/bin/moc $(DEFINES) $(INCPATH) peermanager.hh -o moc_peermanager.cpp

moc_peer.cpp: peer.hh
	/usr/lib64/qt4/bin/moc $(DEFINES) $(INCPATH) peer.hh -o moc_peer.cpp

moc_filemanager.cpp: fileconnection.hh \
		filemanager.hh
	/usr/lib64/qt4/bin/moc $(DEFINES) $(INCPATH) filemanager.hh -o moc_filemanager.cpp

moc_fileconnection.cpp: fileconnection.hh
	/usr/lib64/qt4/bin/moc $(DEFINES) $(INCPATH) fileconnection.hh -o moc_fileconnection.cpp

moc_shamir.cpp: shamir.hh
	/usr/lib64/qt4/bin/moc $(DEFINES) $(INCPATH) shamir.hh -o moc_shamir.cpp

moc_secretmanager.cpp: secretmanager.hh
	/usr/lib64/qt4/bin/moc $(DEFINES) $(INCPATH) secretmanager.hh -o moc_secretmanager.cpp

moc_dht.cpp: peer.hh \
		dht.hh
	/usr/lib64/qt4/bin/moc $(DEFINES) $(INCPATH) dht.hh -o moc_dht.cpp

compiler_rcc_make_all:
compiler_rcc_clean:
compiler_image_collection_make_all: qmake_image_collection.cpp
compiler_image_collection_clean:
	-$(DEL_FILE) qmake_image_collection.cpp
compiler_moc_source_make_all:
compiler_moc_source_clean:
compiler_uic_make_all:
compiler_uic_clean:
compiler_yacc_decl_make_all:
compiler_yacc_decl_clean:
compiler_yacc_impl_make_all:
compiler_yacc_impl_clean:
compiler_lex_make_all:
compiler_lex_clean:
compiler_clean: compiler_moc_header_clean 

####### Compile

main.o: main.cc main.hh \
		netsocket.hh \
		peer.hh \
		peermanager.hh \
		filemanager.hh \
		fileconnection.hh \
		secretmanager.hh \
		dht.hh
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o main.o main.cc

netsocket.o: netsocket.cc netsocket.hh \
		peer.hh \
		peermanager.hh \
		filemanager.hh \
		fileconnection.hh \
		secretmanager.hh \
		dht.hh \
		shamir.hh
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o netsocket.o netsocket.cc

peermanager.o: peermanager.cc peermanager.hh \
		peer.hh
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o peermanager.o peermanager.cc

peer.o: peer.cc peer.hh
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o peer.o peer.cc

filemanager.o: filemanager.cc filemanager.hh \
		fileconnection.hh
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o filemanager.o filemanager.cc

fileconnection.o: fileconnection.cc fileconnection.hh
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o fileconnection.o fileconnection.cc

shamir.o: shamir.cc shamir.hh
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o shamir.o shamir.cc

secretmanager.o: secretmanager.cc secretmanager.hh \
		shamir.hh
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o secretmanager.o secretmanager.cc

dht.o: dht.cc dht.hh \
		peer.hh
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o dht.o dht.cc

moc_main.o: moc_main.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_main.o moc_main.cpp

moc_netsocket.o: moc_netsocket.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_netsocket.o moc_netsocket.cpp

moc_peermanager.o: moc_peermanager.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_peermanager.o moc_peermanager.cpp

moc_peer.o: moc_peer.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_peer.o moc_peer.cpp

moc_filemanager.o: moc_filemanager.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_filemanager.o moc_filemanager.cpp

moc_fileconnection.o: moc_fileconnection.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_fileconnection.o moc_fileconnection.cpp

moc_shamir.o: moc_shamir.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_shamir.o moc_shamir.cpp

moc_secretmanager.o: moc_secretmanager.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_secretmanager.o moc_secretmanager.cpp

moc_dht.o: moc_dht.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_dht.o moc_dht.cpp

####### Install

install:   FORCE

uninstall:   FORCE

FORCE:

