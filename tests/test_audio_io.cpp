#include "SampleIO.h"
#include <iostream>
#include <stdexcept>
void check(bool v,const char*m){if(!v)throw std::runtime_error(m);}
juce::MemoryBlock wav(int channels,int frames){
 juce::MemoryOutputStream o;o.write("RIFF",4);o.writeInt(36+frames*channels*2);o.write("WAVEfmt ",8);o.writeInt(16);o.writeShort(1);o.writeShort(short(channels));o.writeInt(48000);o.writeInt(48000*channels*2);o.writeShort(short(channels*2));o.writeShort(16);o.write("data",4);o.writeInt(frames*channels*2);
 for(int i=0;i<frames;++i){o.writeShort(16384);if(channels==2)o.writeShort(-8192);}return o.getMemoryBlock();
}
int main(){
 std::unique_ptr<anks::Sample>s;
 check(anks::decodeSample(wav(1,1000),s).isEmpty(),"mono decode");check(s->mono.size()==1000&&s->rate==48000,"metadata");check(std::abs(s->mono[500]-0.5)<1e-5,"mono amplitude");check(std::abs(s->mono.front()-0.5)<1e-5&&std::abs(s->mono.back()-0.5)<1e-5,"transient amplitude preserved");
 check(anks::decodeSample(wav(2,1000),s).isEmpty(),"stereo decode");check(std::abs(s->mono[500]-0.125)<1e-5,"downmix");
 check(anks::decodeSample(wav(1,2),s).isEmpty(),"tiny sample");check(std::isfinite(s->mono[0]),"tiny finite output");
 auto bytes=wav(1,1000);juce::MemoryBlock restored;check(restored.fromBase64Encoding(bytes.toBase64Encoding()),"state binary encoding");check(bytes==restored,"state binary roundtrip");
 check(anks::decodeSample(juce::MemoryBlock{},s).isNotEmpty(),"empty accepted");juce::MemoryBlock invalid("invalid",7);check(anks::decodeSample(invalid,s).isNotEmpty(),"invalid accepted");
 check(anks::decodeSample(wav(1,480001),s).isNotEmpty(),"duration limit");
 std::cout<<"PASS: WAV mono/stereo, downmix, transient preservation, tiny sample, embedded-byte roundtrip, invalid files and duration limit.\n";
}
