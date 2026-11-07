#pragma once
#include <juce_audio_formats/juce_audio_formats.h>
#include "Engine.h"
namespace anks {
inline juce::String decodeSample(const juce::MemoryBlock&bytes,std::unique_ptr<anks::Sample>&result){
 if(bytes.getSize()==0||bytes.getSize()>16*1024*1024)return "Fichier vide ou superieur a 16 Mo.";
 juce::AudioFormatManager formats;formats.registerBasicFormats();
 std::unique_ptr<juce::AudioFormatReader> reader(formats.createReaderFor(std::make_unique<juce::MemoryInputStream>(bytes,false)));
 if(!reader)return "Format audio illisible. Utiliser WAV ou AIFF.";
 if(reader->sampleRate<8000||reader->sampleRate>192000||reader->lengthInSamples<2||reader->lengthInSamples>reader->sampleRate*10||reader->numChannels<1||reader->numChannels>2)return "Excitation : 10 secondes maximum, mono/stereo, 8 a 192 kHz.";
 const int n=int(reader->lengthInSamples);juce::AudioBuffer<float>buffer(int(reader->numChannels),n);
 if(!reader->read(&buffer,0,n,0,true,true))return "Lecture audio incomplete.";
 auto s=std::make_unique<anks::Sample>();s->rate=reader->sampleRate;s->mono.resize(size_t(n));
 for(int i=0;i<n;++i){double x=buffer.getSample(0,i);if(buffer.getNumChannels()==2)x=(x+buffer.getSample(1,i))*0.5;if(!std::isfinite(x))return "Le fichier contient des valeurs audio invalides.";s->mono[size_t(i)]=float(std::clamp(x,-1.0,1.0));}
 result=std::move(s);return {};
}
}
