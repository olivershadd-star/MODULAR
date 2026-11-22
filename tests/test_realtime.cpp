#include "Engine.h"
#include <cstdlib>
#include <new>
#include <iostream>
#include <memory>
#include <stdexcept>
static thread_local bool track=false;
static thread_local size_t allocations=0;
void*operator new(std::size_t n){if(track)++allocations;if(auto*p=std::malloc(n?n:1))return p;throw std::bad_alloc();}
void*operator new[](std::size_t n){return ::operator new(n);}
void operator delete(void*p)noexcept{std::free(p);}
void operator delete[](void*p)noexcept{std::free(p);}
void operator delete(void*p,std::size_t)noexcept{std::free(p);}
void operator delete[](void*p,std::size_t)noexcept{std::free(p);}
void check(bool ok,const char*msg){if(!ok)throw std::runtime_error(msg);}
int main(){
 // Compare a reused voice with a fresh voice for every module, including freeze.
 auto used=std::make_unique<anks::Engine>();auto fresh=std::make_unique<anks::Engine>();
 for(int type=1;type<=13;++type){anks::Parameters p;p.rack.enabled=true;p.waveform=1;p.rack.modules[0].type=type;p.rack.modules[0].freezeHold=true;p.rack.modules[0].mix=1;used->parameters=fresh->parameters=p;used->prepare(48000);fresh->prepare(48000);used->noteOn(72,1,.04);for(int i=0;i<4096;++i){if(i%128==0)used->beginBlock();used->nextStereo();}used->panic();used->noteOn(60,1,.04);fresh->noteOn(60,1,.04);for(int i=0;i<4096;++i){if(i%128==0){used->beginBlock();fresh->beginBlock();}auto a=used->nextStereo(),b=fresh->nextStereo();check(std::abs(a.left-b.left)<1e-12&&std::abs(a.right-b.right)<1e-12,"reused voice leaked module state");}}
 // Shared tuning coefficients must never share the audio state of stereo channels.
 anks::StereoRackModule stereo;stereo.prepare(48000);anks::RackParameters resonance;resonance.type=1;resonance.mix=1;double leftEnergy=0;
 for(int i=0;i<12000;++i){if(i%128==0){resonance.body[0].brightness=.4+.2*std::sin(i*.001);stereo.configure(resonance,220,120,0,i==0);}auto y=stereo.next({.1*std::sin(2*anks::pi*220*i/48000),0});leftEnergy+=y.left*y.left;check(y.right==0,"shared tuning leaked left audio into right");}check(leftEnergy>1,"stereo resonator fixture silent");
 // Hold/release/re-capture between spectral hops must not detune a stationary tone.
 anks::SpectralHold freeze;freeze.prepare();double error=0;
 for(int i=0;i<48000;++i){bool hold=(i%2001)>2;double y=freeze.next(.2*std::sin(2*anks::pi*750*i/48000),hold);if(i>4096)error=std::max(error,std::abs(y-.2*std::sin(2*anks::pi*750*(i-1024)/48000)));}
 check(error<1e-8,"rapid spectral recapture changed tone phase");
 // Preparation is outside the callback. MIDI bursts and live topology must not allocate.
 used->parameters=anks::Parameters{};used->parameters.rack.enabled=true;used->parameters.fm.enabled=true;used->prepare(48000);allocations=0;track=true;bool finite=true;
 for(int block=0;block<40;++block){for(int j=0;j<4;++j){auto&m=used->parameters.rack.modules[size_t(j)];m.type=(block+j)%10;m.body[0].brightness=.3+.01*block;m.freezeHold=block%3!=0;}used->parameters.rack.order=block%24;if(block%4==0)for(int v=0;v<16;++v)used->noteOn(36+v,1,.02);used->beginBlock();for(int i=0;i<512;++i){auto y=used->nextStereo();finite=finite&&std::isfinite(y.left)&&std::isfinite(y.right);}if(block%4==3)for(int v=0;v<16;++v)used->noteOff(36+v,1);}
 track=false;check(finite,"live topology nonfinite");check(allocations==0,"audio rendering allocated memory");
 std::cout<<"PASS: reset equivalence for 13 modules, rapid spectral recapture, no allocations during note bursts/topology/render.\n";
}
