################################################################################
#
#  Makefile
#
#  This is the common part of the makefile that builds the TraceTool
#
################################################################################
################################################################################
#
#   FxEngine Framework. 
#	Copyright (c) 2005, 2009 Sylvain Machel, SMProcess.
#
#	This file is part of FxEngine Framework.
#   TraceTool is free software: you can redistribute it and/or modify       
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or   
#	(at your option) any later version.                                 
#                                                                    
#	TraceTool is distributed in the hope that it will be useful,        
#	but WITHOUT ANY WARRANTY; without even the implied warranty of      
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the       
#	GNU General Public License for more details.                        
#                                                                    
#	You should have received a copy of the GNU General Public License   
#	along with TraceTool. If not, see <http://www.gnu.org/licenses/>.   
#	if not,  If not, see <http://www.gnu.org/licenses/>.
#
################################################################################
#Define Compiler
CCC=gcc

#define Target
TARGET_NAME=TraceTool

#define source path
SOURCE=../../Src

.SUFFIXES: .o .cpp

OBJECTS=$(SOURCE)/TraceTool.o

#define C Flags
CFLAGS=-O3 -I$(SOURCE)

#define Lib Flags
LFLAGS=-lstdc++

.cpp.o:
	@echo Compiling $<
	@$(CCC) $(CFLAGS) -c $< -o $@

all: $(OBJECTS)
	@echo Linking...
	@$(CCC) $(CFLAGS) $(LFLAGS) $(OBJECTS) -o $(TARGET_NAME)
	@echo Copying...
	
clean:
	@echo Cleaning...
	@if ( test -f *.o ) then rm *.o; fi
	@if ( test -f $(SOURCE)/*.o ) then rm $(SOURCE)/*.o; fi
	@if ( test -f *.so* ) then rm *.so*; fi
