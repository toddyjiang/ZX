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
################################################################################
#
#  Build All projects
#
################################################################################
#Define FxEngine Framework
FXENGINE=../../Src/Build/x86-linux
cd $FXENGINE;
make clean; make;
cd -

#define FxEngine Samples
FXENGINE_SAMPLE_HELLO=../../Liv/Samples/FxHelloWorldSrc/Build/x86-linux/
cd $FXENGINE_SAMPLE_HELLO; make clean; make;
cd -

FXENGINE_SAMPLE_NULL=../../Liv/Samples/FxNullRnd/Build/x86-linux/
cd $FXENGINE_SAMPLE_NULL; make clean; make;
cd -

FXENGINE_SAMPLE_FILE_SRC=../../Liv/Samples/FxSndFileSrc/Build/x86-linux/
cd $FXENGINE_SAMPLE_FILE_SRC; make clean; make;
cd -

FXENGINE_SAMPLE_PCM_GAIN=../../Liv/Samples/FxPcmGain/Build/x86-linux/
cd $FXENGINE_SAMPLE_PCM_GAIN; make clean; make;
cd -

FXENGINE_SAMPLE_WAV_RND=../../Liv/Samples/FxWavFileRnd/Build/x86-linux/
cd $FXENGINE_SAMPLE_WAV_RND; make clean; make;
cd -

FXENGINE_SAMPLE_LADSPA=../../Liv/Samples/FxLADSPA/Build/x86-linux/
cd $FXENGINE_SAMPLE_LADSPA; make clean; make;
cd -

FXENGINE_SAMPLE_ALSA_RND=../../Liv/Samples/FxAlsaRnd/Build/x86-linux/
cd $FXENGINE_SAMPLE_ALSA_RND; make clean; make;
cd -

FXENGINE_SAMPLE_MTOS=../../Liv/Samples/FxMToS/Build/x86-linux/
cd $FXENGINE_SAMPLE_MTOS; make clean; make;
cd -

FXENGINE_SAMPLE_STOM=../../Liv/Samples/FxSToM/Build/x86-linux/
cd $FXENGINE_SAMPLE_STOM; make clean; make;
cd -

FXENGINE_SAMPLE_STOM_SPLITTER=../../Liv/Samples/FxSToMSplitter/Build/x86-linux/
cd $FXENGINE_SAMPLE_STOM_SPLITTER; make clean; make;
cd -

FXENGINE_SAMPLE_MTOS_MERGER=../../Liv/Samples/FxMToSMerger/Build/x86-linux/
cd $FXENGINE_SAMPLE_MTOS_MERGER; make clean; make;
cd -

#define FxEngine Editor
FXENGINE_EDITOR=../../Tools/FxEngineEditor/Build/x86-linux/
cd $FXENGINE_EDITOR; make clean; make;
cd -

#define FxEngine Trace Tool
FXENGINE_TRACE_TOOL=../../Tools/TraceTool/Build/x86-linux/
cd $FXENGINE_TRACE_TOOL; make clean; make;
cd -




 
