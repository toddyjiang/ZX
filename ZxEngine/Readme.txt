FxEngine Framework Copyright (C) 2005-2009 Sylvain Machel, SMProcess.

FxEngine Framework Contents:
============================

Build/x86-linux/ ....Contains the main build batch. It will build libraries, Samples and Tools.
					See Build.txt for more details on FxEngine Framework compilation.
Liv/ ....Delivery directory. Contains the FxEngine framework files ready to be deployed. 
	Bin/ ....Contains the FxEngine Framework Library files, samples and the FxEngine Editor application.
	Include/ ...Contains FxEngine Framework include files for FxEngine core and FX's components.
	Lib/ ....Contains FxEngine Framework library files for FxEngine core and FX's components.
	Samples/ ....Contains all samples code. Sample binaries are copied in bin directory.
		FxNullRnd/ ....The Fx Null renderer illustrates the simplest renderer Fx.
		FxHelloWorldSrc/ ....The Fx HelloWorld source sends "Hello World" text on its output pin. 
		FxSndFileSrc/ ....The Fx Sound File source reads sound data from file and sends PCM data on its output pin.
					  this Fx uses the libsndfile library from Erik de Castro Lopo. See http://www.mega-nerd.com/libsndfile
		FxPcmGain/ ....The Fx Pcm Gain applies a gain on sound data from its input pin. 
		FxLADSPA/ ....This Fx is a LDSPA wrapper. It allows to load most of popular LADSPA plugins. 
		FxWavFileRnd/ ....The Fx wave File renderer get pcm data on its input pin and creates a wav file.
					This Fx uses the libsndfile library from Erik de Castro Lopo. See http://www.mega-nerd.com/libsndfile
		FxAlsaRnd/ ....This Fx is an audio renderer. It allows to play audio stream using Alsa sound driver. 
		FxMToS/ ....This Fx transforms its input audio stream (Mono or Stereo) to a stereo audio stream. 
		FxSToM/ ....This Fx transforms its input audio stream (Mono or Stereo) to a mono audio stream. 	
		FxSToMSplitter/ ....This Fx transforms its input audio stream (Mono or Stereo) to two mono audio streams.
		FxMToSMerger/ ....This Fx transforms its both input audio streams (Mono or Stereo) to one stereo audio streams.	
Doc/ ....Contains the FxEngine Framework documentation in PDF format.	
Src/
	Build/x86-linux ....Contains the FxEngine Framework build batch.
	Include/ ....Contains FxEngine Framework .h files. 
	Src/ ....Contains FxEngine Framework .cpp files. 
Tools/
	FxEngineEditor/ ....FxEngineEditor is the graphical editor to handle Fx.
		Build/x86-linux ....Contains the FxEngine Editor build batch.
		Src/ ....Contains FxEngine Editor .cpp files. 
	TraceTool/ ....TraceTool is a tool to enable the FxEngine Framework traces.
		Build/x86-linux ....Contains the TraceTool build batch.
		Src/ ....Contains TraceTool .cpp files.
