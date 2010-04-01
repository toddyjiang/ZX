################################################################################
#
#  Makefile
#
################################################################################
################################################################################
#
#   FxEngine Framework. 
#	Copyright (c) 2005, 2009 Sylvain Machel, SMProcess.
#
#	This file is part of FxEngine Framework.
#   The FxEngine Framework library is free software; you can redistribute it
#	and/or modify it under the terms of the GNU Lesser General Public
#	License as published by the Free Software Foundation; either
#	version 2.1 of the License, or (at your option) any later version.
#
#	This library is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#	Lesser General Public License for more details.
#
#	You should have received a copy of the GNU Lesser General Public
#	License along with this library.
#	if not,  If not, see <http://www.gnu.org/licenses/>.
#
################################################################################
#Define Compiler
CCC=g++

#define Target
TARGET_NAME=libFxEngine
TARGET_MAIN_VERSION=4
TARGET_SUB_VERSION=4
TARGET_PATH=../../../Liv/Lib
TARGET=$(TARGET_NAME).so.$(TARGET_MAIN_VERSION) -o $(TARGET_PATH)/$(TARGET_NAME).so.$(TARGET_MAIN_VERSION).$(TARGET_SUB_VERSION)

TARGET_CP_INC=../../../Liv/Include

#define source path
SRC_PATH=../../Src/
SRC_CORE=$(SRC_PATH)/Core
SRC_FXCONST2STRING=$(SRC_PATH)/FxConst2String
SRC_FXMATRIXFORMAT=$(SRC_PATH)/FxMatrixFormat
SRC_FXMEDIA=$(SRC_PATH)/FxMedia
SRC_FXMEDIAPOOL=$(SRC_PATH)/FxMediaPool
SRC_FXPARAM=$(SRC_PATH)/FxParam
SRC_FXPCMFORMAT=$(SRC_PATH)/FxPcmFormat
SRC_FXPIN=$(SRC_PATH)/FxPin
SRC_FXPINMANAGER=$(SRC_PATH)/FxPinManager
SRC_FXREFCLOCK=$(SRC_PATH)/FxRefClock
SRC_FXSTATE=$(SRC_PATH)/FxState
SRC_FXTRACE=$(SRC_PATH)/FxTrace
SRC_FXUTILS=$(SRC_PATH)/FxUtils
SRC_FXVECTORFORMAT=$(SRC_PATH)/FxVectorFormat
SRC_FXVIDEOIMGFORMAT=$(SRC_PATH)/FxVideoImgFormat
SRC_FXXML=$(SRC_PATH)/FxXML

#define Interface path
INC_INTERFACE=../../Include
INC_SRC=../../Src

#define BOOST library path
LIB_BOOST=../../Lib
INC_BOOST=../../../../Boost/boost_1_34_1

.SUFFIXES: .o .cpp

OBJECTS=$(SRC_CORE)/FxEngine.o \
$(SRC_CORE)/FxEngineCore.o \
$(SRC_CORE)/FxFactory.o \
$(SRC_CORE)/Fx.o \
$(SRC_FXCONST2STRING)/FxConst2String.o \
$(SRC_FXMATRIXFORMAT)/FxMatrixFormat.o \
$(SRC_FXMEDIA)/FxMedia.o \
$(SRC_FXMEDIAPOOL)/FxMediaPool.o \
$(SRC_FXPARAM)/FxParam.o \
$(SRC_FXPCMFORMAT)/FxPcmFormat.o \
$(SRC_FXPIN)/FxPin.o \
$(SRC_FXPINMANAGER)/FxPinManager.o \
$(SRC_FXREFCLOCK)/FxRefClock.o \
$(SRC_FXSTATE)/FxState.o \
$(SRC_FXSTATE)/State.o \
$(SRC_FXTRACE)/FxTrace.o \
$(SRC_FXUTILS)/FxAPC.o \
$(SRC_FXVECTORFORMAT)/FxVectorFormat.o \
$(SRC_FXVIDEOIMGFORMAT)/FxVideoImgFormat.o \
$(SRC_FXXML)/XMLConnection.o \
$(SRC_FXXML)/XMLFx.o \
$(SRC_FXXML)/XMLRoot.o

#define CPP Flags
CFLAGS=-Wall -fPIC -D__i386__ -O3 -I$(INC_INTERFACE) -I$(INC_SRC) -I$(INC_BOOST)

#define Libraries Flags
LDFLAGS=-s -fPC -ldl -lpthread -shared -Wl,-soname,$(TARGET_NAME).so.$(TARGET_MAIN_VERSION)

.cpp.o:
	@echo Compiling... $<
	@$(CCC) $(CFLAGS) -c $< -o $@

all: $(OBJECTS)
	@echo Linking ...
	@$(CCC) $(CFLAGS) $(LDFLAGS) -o $(TARGET_PATH)/$(TARGET_NAME).so.$(TARGET_MAIN_VERSION).$(TARGET_SUB_VERSION) $(OBJECTS) -L$(LIB_BOOST) -lboost_thread-gcc41-mt-1_34_1
	@echo Copying...
	@cp ${INC_INTERFACE}/*.h ${TARGET_CP_INC} 
	@ln -sf $(TARGET_NAME).so.$(TARGET_MAIN_VERSION).$(TARGET_SUB_VERSION) $(TARGET_PATH)/$(TARGET_NAME).so.$(TARGET_MAIN_VERSION)
	@ln -sf $(TARGET_NAME).so.$(TARGET_MAIN_VERSION) $(TARGET_PATH)/$(TARGET_NAME).so
clean:
	@echo Cleaning...
	@rm -f $(OBJECTS)
 
