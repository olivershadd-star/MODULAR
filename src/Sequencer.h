#pragma once
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>
namespace anks {
struct SequenceStep{int note=36,velocity=100;double gate=0.6;bool enabled=true;};
struct SequenceConfig{
 bool enabled=false,internal=false,run=false;double bpm=120;
 int length=16,division=2,channel=1;
 std::array<SequenceStep,16>steps{};
};
struct HostClock{bool valid=false,playing=false;double bpm=120,ppq=0;};
struct SequenceEvent{bool on=false;int note=36,channel=1;double velocity=1;};
struct SequenceEvents{std::array<SequenceEvent,2>items{};int count=0;};
// A fixed-capacity, sample-stepped clock. No allocations and no wall-clock timers.
class StepSequencer{
public:
 void prepare(double sampleRate){rate=sampleRate;reset();}
 void reset(){started=false;active=false;owned=false;pendingStop=false;position=expected=0;ordinal=0;display=-1;}
 void beginBlock(const SequenceConfig&next,const HostClock&host){
  bool changed=config.internal!=next.internal||config.length!=next.length||config.division!=next.division||config.channel!=next.channel;
  bool modeChanged=config.internal!=next.internal;
  config=next;config.length=std::clamp(config.length,1,16);config.division=std::clamp(config.division,0,3);config.channel=std::clamp(config.channel,1,16);
  active=config.enabled&&(config.internal?config.run:(host.valid&&host.playing));
  if(!active){pendingStop=owned;started=false;display=-1;return;}
  double bpm=config.internal?config.bpm:host.bpm;
  if(!std::isfinite(bpm)||bpm<=0||rate<=0){active=false;pendingStop=owned;started=false;display=-1;return;}
  increment=bpm/(60*rate);quarterPerStep=std::ldexp(1.0,-config.division);
  bool discontinuity=!started||changed;
  if(config.internal){if(!started||modeChanged)position=0;}
  else{
   if(!std::isfinite(host.ppq)||std::abs(host.ppq)>1e9){active=false;pendingStop=owned;started=false;display=-1;return;}
   if(started&&std::abs(host.ppq-expected)>std::max(1e-7,increment*2))discontinuity=true;
   position=host.ppq;
  }
  if(discontinuity){pendingStop=owned;ordinal=int64_t(std::ceil((position-1e-10)/quarterPerStep));display=-1;}
  started=true;
 }
 SequenceEvents tick(){
  SequenceEvents out;
  auto stop=[&]{if(owned){out.items[size_t(out.count++)]={false,ownedNote,ownedChannel,0};owned=false;}};
  if(pendingStop){stop();pendingStop=false;}
  if(!active)return out;
  if(owned&&position+1e-10>=offAt)stop();
  double boundary=double(ordinal)*quarterPerStep;
  if(position+1e-10>=boundary){
   stop();display=int((ordinal%config.length+config.length)%config.length);
   const auto&s=config.steps[size_t(display)];
   if(s.enabled){owned=true;ownedNote=std::clamp(s.note,0,127);ownedChannel=config.channel;out.items[size_t(out.count++)]={true,ownedNote,ownedChannel,std::clamp(s.velocity,1,127)/127.0};offAt=boundary+std::clamp(s.gate,0.05,1.0)*quarterPerStep;}
   ++ordinal;
  }
  position+=increment;expected=position;
  return out;
 }
 int currentStep()const{return display;}
 bool isActive()const{return active;}
private:
 SequenceConfig config;double rate=48000,position=0,expected=0,increment=0,quarterPerStep=.25,offAt=0;
 int64_t ordinal=0;int display=-1,ownedNote=36,ownedChannel=1;
 bool started=false,active=false,owned=false,pendingStop=false;
};
}
