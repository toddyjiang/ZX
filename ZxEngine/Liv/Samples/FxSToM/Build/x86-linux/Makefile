################################################################################
#
#  Makefile
#
#  This is the common part of the makefile that builds the FxSToM.so
#
################################################################################

#Define Compiler
CCC=gcc

#define Target
TARGET_NAME=libFxSToM
TARGET=$(TARGET_NAME).so -o $(TARGET_NAME).so

#define source path
FX_SOURCE=../../Src
COMMON_INCLUDE=../../../../Include

#define FxEngine library path
FXENGINE_LIB=../../../../Lib

#define FxEngine bin path
FXENGINE_BIN=../../../../Bin/

.SUFFIXES: .o .cpp

OBJECTS=$(FX_SOURCE)/FxSToM.o

#define C Flags
CFLAGS=-fPIC -O3 -I$(FX_SOURCE) -I$(COMMON_INCLUDE)

#define Libraries Flags
LDFLAGS=-shared -Wl,-soname,$(TARGET) -lFxEngine -L$(FXENGINE_LIB)

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

 
