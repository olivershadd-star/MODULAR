#include "Sequencer.h"
#include "Engine.h"
#include <vector>
#include <memory>
#include <stdexcept>
#include <iostream>
void check(bool b,const char*m){if(!b)throw std::runtime_error(m);}
struct Event{int64_t sample;anks::SequenceEvent e;};
std::vector<Event>run(anks::SequenceConfig p,double sr,int total,int block,bool host){
 anks::StepSequencer seq;seq.prepare(sr);p.enabled=true;p.internal=!host;p.run=true;
 std::vector<Event>events;
 for(int start=0;start<total;start+=block){anks::HostClock c;c.valid=true;c.playing=true;c.bpm=p.bpm;c.ppq=start*p.bpm/(60*sr);seq.beginBlock(p,c);for(int i=0;i<std::min(block,total-start);++i){auto out=seq.tick();check(out.count<=2,"event capacity");for(int j=0;j<out.count;++j)events.push_back({start+i,out.items[size_t(j)]});}}
 return events;
}
int main(){
 anks::SequenceConfig p;p.length=4;p.bpm=120;p.division=2;for(int i=0;i<16;++i){p.steps[size_t(i)].note=36+i;p.steps[size_t(i)].gate=.5;}
 auto ref=run(p,48000,25000,512,true);std::vector<Event>ons,offs;for(auto&e:ref)(e.e.on?ons:offs).push_back(e);
 check(ons.size()==5,"host note count");for(size_t i=0;i<ons.size();++i){check(ons[i].sample==int64_t(i)*6000,"host step timing");check(ons[i].e.note==36+int(i%4),"step wrap");if(i<offs.size())check(offs[i].sample==int64_t(i)*6000+3000,"gate timing");}
 for(int block:{1,32,127,512,1024}){auto r=run(p,48000,25000,block,true);check(r.size()==ref.size(),"block event count");for(size_t i=0;i<r.size();++i)check(r[i].sample==ref[i].sample&&r[i].e.note==ref[i].e.note&&r[i].e.on==ref[i].e.on,"block-size invariance");}
 auto local=run(p,48000,25000,32,false);check(local.size()==ref.size(),"internal count");for(size_t i=0;i<ref.size();++i)check(local[i].sample==ref[i].sample,"internal clock");
 for(double sr:{44100.,48000.,96000.})for(double bpm:{73.,127.,160.}){p.bpm=bpm;auto e=run(p,sr,int(sr*2),32,true);int64_t n=0;for(auto&v:e)if(v.e.on){double exact=double(n)*sr*60/bpm*.25;check(std::abs(double(v.sample)-exact)<1.001,"noninteger grid drift");++n;}}
 p.bpm=120;p.steps[1].enabled=false;p.steps[0].gate=1;auto muted=run(p,48000,13000,32,true);bool at6000on=false,at6000off=false;for(auto&e:muted){if(e.sample==6000&&e.e.on)at6000on=true;if(e.sample==6000&&!e.e.on)at6000off=true;}check(!at6000on&&at6000off,"rest/full gate");
 // Stop, missing host, loop/seek and phase-aligned restart.
 anks::StepSequencer seq;seq.prepare(48000);p.enabled=true;p.internal=false;p.length=16;p.steps[0].gate=1;anks::HostClock c;c.valid=true;c.playing=true;c.bpm=120;c.ppq=0;seq.beginBlock(p,c);check(seq.tick().count==1,"start trigger");c.playing=false;seq.beginBlock(p,c);auto stop=seq.tick();check(stop.count==1&&!stop.items[0].on,"transport stop off");check(seq.tick().count==0,"repeated stop");
 c.playing=true;c.ppq=.1;seq.beginBlock(p,c);check(seq.tick().count==0,"restart should wait next boundary");
 c.ppq=0;seq.beginBlock(p,c);check(seq.tick().items[0].on,"seek to start");c.valid=false;seq.beginBlock(p,c);stop=seq.tick();check(stop.count==1&&!stop.items[0].on,"missing host off");
 p.internal=true;p.run=true;seq.beginBlock(p,c);check(seq.tick().items[0].on,"internal independent of host");p.enabled=false;seq.beginBlock(p,c);check(!seq.tick().items[0].on,"disable off");
 // Incoming notes and sequence notes of the same pitch must not steal each other's gate.
 auto engine=std::make_unique<anks::Engine>();engine->prepare(48000);engine->parameters.release=.01;engine->noteOn(60,1,1,0);engine->noteOn(60,1,1,1);engine->setSustain(1,true);engine->noteOff(60,1,1);for(int i=0;i<1000;++i)engine->next();check(engine->activeVoices()==1,"sequence off cut live note or obeyed pedal");engine->setSustain(1,false);engine->noteOff(60,1,0);for(int i=0;i<1000;++i)engine->next();check(engine->activeVoices()==0,"live note off");
 std::cout<<"PASS: host/internal clocks, 32-sample rack chunks, block invariance, tempo/rate grids, gates/rests, stop/seek/missing host, live/sequence ownership.\n";
}
