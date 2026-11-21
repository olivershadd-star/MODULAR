#include "Engine.h"
#include <iostream>
#include <stdexcept>
#include <memory>
void check(bool ok,const char*msg){if(!ok)throw std::runtime_error(msg);}
int main(){
 for(int type=1;type<=9;++type){auto module=std::make_unique<anks::RackModule>();anks::RackParameters p;p.type=type;p.mix=1;p.freezeHold=true;module->prepare(48000);module->configure(p,220,120,0,true);module->setGate(true);for(int i=0;i<12000;++i)module->next(.1*std::sin(2*anks::pi*220*i/48000));p.on=false;module->configure(p,220,120,.5);for(int i=0;i<9000;++i)module->next(.1);check(module->isSuspended(),"OFF did not suspend DSP");for(int i=0;i<256;++i)check(module->next(.123)==.123,"OFF is not transparent");p.on=true;module->configure(p,220,120,.7);module->setGate(true);check(!module->isSuspended(),"ON did not resume DSP");for(int i=0;i<2048;++i)check(std::abs(module->next(0))<1e-9,"old effect state leaked on resume");p.on=false;module->reset();module->configure(p,220,120,0,true);check(module->isSuspended(),"new OFF voice still runs DSP");}
 // A captured ramp exposes the discontinuity of a hard loop boundary.
 auto module=std::make_unique<anks::RackModule>();module->prepare(48000);anks::RackParameters p;p.type=4;p.mix=1;p.grainSize=.25;p.density=1;p.scatter=0;p.position=.3;module->configure(p,220,120,0,true);double previous=0,jump=0;for(int i=0;i<8000;++i){double input=i<240?-.2+.4*i/239.:.2;double y=module->next(input);if(i>1000)jump=std::max(jump,std::abs(y-previous));previous=y;check(std::isfinite(y),"granular seam nonfinite");}std::cout<<"Granular seam maximum step: "<<jump<<'\n';check(jump<.03,"granular fragment boundary clicks");
 // Extreme scatter must not select a two-sample fragment for a long grain.
 p.scatter=1;p.pitch=24;p.density=64;module->reset();module->configure(p,220,120,0,true);for(int i=0;i<12000;++i)check(std::isfinite(module->next(.1*std::sin(2*anks::pi*220*i/48000))),"granular scatter unsafe");
 std::cout<<"PASS: nine true bypass/resume paths, transparent OFF, clean restart, continuous fragment seams and scatter bounds.\n";
}
