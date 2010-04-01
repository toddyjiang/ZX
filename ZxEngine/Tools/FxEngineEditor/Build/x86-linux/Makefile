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
#	FxEngine Editor is free software: you can redistribute it and/or modify   
#   it under the terms of the GNU General Public License as published by  
#   the Free Software Foundation, either version 3 of the License, or     
#   (at your option) any later version.                                   
#                                                                         
#   FxEngine Editor is distributed in the hope that it will be useful,    
#   but WITHOUT ANY WARRANTY; without even the implied warranty of        
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         
#   GNU General Public License for more details.                          
#                                                                         
#   You should have received a copy of the GNU General Public License     
#   along with FxEngine Editor. If not, see <http://www.gnu.org/licenses/>.  
#	along with TraceTool. If not, see <http://www.gnu.org/licenses/>.           
#	if not,  If not, see <http://www.gnu.org/licenses/>.                        
#                                                                               
################################################################################

#Define Compiler
CCC=gcc

#define Target
TARGET_NAME=FxEngineEditor
TARGET_PATH=../../../../Liv/Bin

#define source path
SRC_PATH=../../Src/

#define Interface path
INC_SRC=../../Src/

#define FxEngine path
LIB_FXENGINE=../../../../Liv/Lib
INC_FXENGINE=../../../../Liv/Include

#define BOOST library path
INC_BOOST=../../../../../Boost/boost_1_34_1
LIB_BOOST=../../../../Src/Lib

.SUFFIXES: .o .cpp

OBJECTS=$(SRC_PATH)/XMLRoot.o \
$(SRC_PATH)/XMLOptions.o \
$(SRC_PATH)/XMLDir.o \
$(SRC_PATH)/NodeManager.o \
$(SRC_PATH)/Node.o \
$(SRC_PATH)/FxPropertySheet.o \
$(SRC_PATH)/FxLine.o \
$(SRC_PATH)/FxEngineEditorView.o \
$(SRC_PATH)/FxEngineEditorDoc.o \
$(SRC_PATH)/FxEngineEditor.o \
$(SRC_PATH)/FxColor.o \
$(SRC_PATH)/Fx.o \
$(SRC_PATH)/EditorPropertySheet.o \
$(SRC_PATH)/CTreeItemData.o \
$(SRC_PATH)/CTreeCtrl.o \
$(SRC_PATH)/AdjustNode.o

#define CPP Flags
CFLAGS=-Wall -D__i386__ -O3 -D__WXGTK__ `wx-config --cflags` -I$(INC_SRC) -I$(INC_FXENGINE) -I$(INC_BOOST)
#define Libraries Flags
LDFLAGS=-s -lFxEngine -L$(LIB_FXENGINE)

.cpp.o:
	@echo Compiling... $<
	@$(CCC) $(CFLAGS) -c $< -o $@

all: $(OBJECTS)
	@echo Linking ...
	@$(CCC) $(CFLAGS) $(LDFLAGS) -o $(TARGET_NAME) $(OBJECTS) `wx-config --libs`
	@echo Copying...
	
	@cp $(TARGET_NAME) $(TARGET_PATH)
	
clean:
	@echo Cleaning...
	@rm -f $(OBJECTS)
 
