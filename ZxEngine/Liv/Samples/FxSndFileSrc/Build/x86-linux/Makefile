################################################################################
#
#  Makefile
#
#  This is the common part of the makefile that builds the FxSndFileSrc.so
#
################################################################################
#
#  This Fx uses the libsndfile library from Erik de Castro Lopo.
#  See http://www.mega-nerd.com/libsndfile
#
################################################################################

#Define Compiler
CCC=gcc

#define Target
TARGET_NAME=libFxSndFileSrc
TARGET=$(TARGET_NAME).so -o $(TARGET_NAME).so

#define sources path
FX_SOURCE=../../Src
COMMON_INCLUDE=../../../../Include
SNDFILE_INCLUDE=/usr/local/include

#define FxEngine libraries path
FXENGINE_LIB=../../../../Lib
SNDFILE_LIB=/usr/local/lib

#define FxEngine bin path
FXENGINE_BIN=../../../../Bin/

.SUFFIXES: .o .cpp

OBJECTS=$(FX_SOURCE)/FxSndFileSrc.o

#define C Flags
CFLAGS=-fPIC -O3 -I$(FX_SOURCE) -I$(COMMON_INCLUDE) -I$(SNDFILE_INCLUDE) `pkg-config --cflags gtk+-2.0`

#define Libraries Flags
LDFLAGS=-shared -Wl,-soname,$(TARGET) -lFxEngine -lsndfile -L$(FXENGINE_LIB) -L$(SNDFILE_LIB) `pkg-config --libs gtk+-2.0`

.cpp.o:
	@echo Compiling... $<
	@$(CCC) $(CFLAGS) -c $< -o $@

all: $(OBJECTS)
	@echo Linking ...
	@$(CCC) $(CFLAGS) $(LDFLAGS) $(OBJECTS)
	@echo Copying...
	@cp ./$(TARGET_NAME).so $(FXENGINE_BIN)

clean:
	@echo Cleaning...
	@if ( test -f *.o ) then rm *.o; fi
	@if ( test -f $(FX_SOURCE)/*.o ) then rm $(FX_SOURCE)/*.o; fi
	@if ( test -f *.so* ) then rm *.so*; fi

 
