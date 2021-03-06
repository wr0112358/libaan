PROJECT_NAME=libaan

MAJOR_ID=0
MINOR_ID=0.1
SOVERSION=$(MAJOR_ID).$(MINOR_ID)
SO_LINKERNAME=$(PROJECT_NAME).so
SO_REALNAME=$(SO_LINKERNAME).$(SOVERSION)
SONAME=$(SO_LINKERNAME).$(MAJOR_ID)

ASAN_CXXFLAGS=-fsanitize=address -fno-omit-frame-pointer

WARN_MORE=-Wextra -Wpedantic -Wpointer-arith -Wcast-align -Wredundant-decls \
	-Wdisabled-optimization -Wno-long-long -Wwrite-strings -Wcast-qual \
	-pedantic-errors -ggdb3
CXXFLAGS=-std=c++1y -Wall -Werror $(WARN_MORE) # $(ASAN_CXXFLAGS)

#LDFLAGS+=-lasan

ifeq ($(CXX_OVERRIDE),)
    CXX=g++
    CC=$(CXX)
else
    CXX=$(CXX_OVERRIDE)
    CC=$(CXX)
endif

ifeq ($(WORDSFILE_OVERRIDE),)
    WORDSFILE=/usr/share/dict/words
else
    WORDSFILE=$(WORDSFILE_OVERRIDE)
endif

# relative..
PROJECT_ROOT:=$(dir $(lastword $(MAKEFILE_LIST)))
# or absolute paths
# PROJECT_ROOT := $(dir $(CURDIR)/$(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST)))

ifneq ("$(wildcard /usr/lib64/)","")
     INSTALL_PATH=/usr/lib64
else ifneq ("$(wildcard /usr/lib/x86_64-linux-gnu/)","")
     INSTALL_PATH=/usr/lib/x86_64-linux-gnu
else
     INSTALL_PATH=/usr/lib
endif


export CXX
export CC
