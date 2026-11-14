#include "Engine.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <chrono>
#include <vector>
void check(bool b,const char*m){if(!b)throw std::runtime_error(m);}
using Buffer=std::vector<double>;
anks::Sample source(){anks::Sample s;s.rate=48000;s.mono.resize(48000);for(size_t i=0;i<s.mono.size();++i)s.mono[i]=float(.13*std::sin(2*anks::pi*220*double(i)/48000)+.07*std::sin(2*anks::pi*1550*double(i)/48000));return s;}
Buffer render(anks::Parameters p,const anks::Sample&s,int n=48000,int block=128){auto e=std::make_unique<anks::Engine>();e->sample=&s;e->parameters=p;e->prepare(48000);e->noteOn(60,1,1);Buffer out;out.reserve(size_t(n));for(int i=0;i<n;++i){if(i%block==0){e->parameters.rack.ppq=double(i)/24000;e->beginBlock();}double v=e->next();check(std::isfinite(v)&&std::abs(v)<=.98,"nonfinite rack output");out.push_back(v);}return out;}
double diff(const Buffer&a,const Buffer&b){double v=0;for(size_t i=0;i<a.size();++i)v+=std::abs(a[i]-b[i]);return v;}
double power(const Buffer&a,int start=0){double v=0;for(size_t i=size_t(start);i<a.size();++i)v+=a[i]*a[i];return v;}
int main(){
 // Compare wrapped interpolation to the original absolute ring definition.
 anks::History history;history.prepare(17);
 for(int pass=0;pass<2;++pass){history.reset();for(int n=1;n<180;++n){history.push(double(n));for(double age:{2.,2.25,5.75,14.}){double pos=n-age;double expected=pos<0||pos<double(std::max(0,n-17))?0:pos+1;check(std::abs(history.absolute(pos)-expected)<1e-6,"wrapped history interpolation");}}}
 auto s=source();anks::Parameters p;p.useSample=true;p.rack.enabled=true;
 auto dry=render(p,s,12000);for(size_t i=0;i<dry.size();++i)check(std::abs(dry[i]-s.mono[i])<1e-7,"empty slot not transparent");
 for(int type=1;type<=6;++type){p.rack.modules[0].type=type;auto changed=render(p,s);check(diff(changed,render(anks::Parameters(p),s,48000,32))<1e-6,"rack block invariance");check(diff(changed,render([&]{auto q=p;q.rack.modules[0].on=false;return q;}(),s))>.01,"module has no effect");check(power(changed)>1e-5,"module silent");}
 p.rack.modules[0].type=1;p.rack.modules[0].mix=1;p.rack.modules[0].balance=.5;
 auto both=render(p,s);auto a=p;a.rack.modules[0].body[0].detune=700;auto b=p;b.rack.modules[0].body[1].detune=-700;check(diff(both,render(a,s))>1&&diff(both,render(b,s))>1,"both resonant bodies must contribute");
 p.rack.modules[1]=p.rack.modules[0];p.rack.modules[1].body[0].position=.4;auto serial=render(p,s);p.rack.modules[1].on=false;check(diff(serial,render(p,s))>1,"second resonator not independent");
 // A low filter before saturation differs from the opposite order.
 p=anks::Parameters{};p.useSample=true;p.rack.enabled=true;p.rack.modules[0].type=2;p.rack.modules[0].mix=1;p.rack.modules[0].cutoff=500;p.rack.modules[1].type=3;p.rack.modules[1].mix=1;p.rack.modules[1].drive=30;
 auto order=render(p,s);for(int i=0;i<24;++i)if(anks::rackOrders()[size_t(i)][0]==1&&anks::rackOrders()[size_t(i)][1]==0){p.rack.order=i;break;}check(diff(order,render(p,s))>1,"order does not affect audio");
 // Equalizer is flat at zero gains and acts when one band is boosted.
 p=anks::Parameters{};p.useSample=true;p.rack.enabled=true;p.rack.modules[0].type=2;p.rack.modules[0].filterMode=4;p.rack.modules[0].mix=1;auto flat=render(p,s,12000);check(diff(flat,dry)<1e-6,"flat EQ changes audio");p.rack.modules[0].eqGain[0]=12;check(diff(flat,render(p,s,12000))>1,"parametric EQ inactive");
 // A granulator really receives preceding processing, not a parallel source reader.
 p=anks::Parameters{};p.useSample=true;p.rack.enabled=true;p.rack.modules[0].type=2;p.rack.modules[0].mix=1;p.rack.modules[0].cutoff=200;p.rack.modules[1].type=4;p.rack.modules[1].mix=1;auto filteredGrains=render(p,s);p.rack.modules[0].on=false;check(diff(filteredGrains,render(p,s))>1,"granulator ignores incoming signal");
 for(int mode=0;mode<5;++mode){p.rack.modules[0].on=true;p.rack.modules[0].filterMode=mode;p.rack.modules[0].q=12;p.rack.modules[0].lfoDepth=5;p.rack.modules[0].lfoRate=20;render(p,s,5000);}
 // Free and musical LFOs must be independent of audio callback size.
 p=anks::Parameters{};p.useSample=true;p.rack.enabled=true;p.rack.modules[0].type=2;p.rack.modules[0].lfoDepth=2;p.rack.modules[0].lfoSync=true;check(diff(render(p,s,24000,32),render(p,s,24000,512))<1e-5,"synced LFO block invariance");
 for(int wave=1;wave<=3;++wave){auto w=p;w.useSample=false;w.waveform=wave;w.rack.modules[0].type=0;check(power(render(w,s,12000))>10,"generated wave silent");}
 // Ring invalidation must not leak an earlier note into a fresh one.
 auto e=std::make_unique<anks::Engine>();e->parameters=p;e->parameters.rack.modules[0].type=5;e->parameters.rack.modules[0].mix=1;e->sample=&s;e->prepare(48000);e->noteOn(60,1,1);for(int i=0;i<20000;++i){if(i%128==0)e->beginBlock();e->next();}e->panic();anks::Sample silence;silence.mono.assign(48000,0);e->sample=&silence;e->noteOn(60,1,1);for(int i=0;i<24000;++i){if(i%128==0)e->beginBlock();check(e->next()==0,"previous voice history leaked");}
 // Inactive architecture automation must not duck or alter the active motor.
 auto inactiveEdit=[&](bool modern,bool edit){auto engine=std::make_unique<anks::Engine>();engine->sample=&s;engine->parameters.useSample=true;engine->parameters.rack.enabled=modern;engine->prepare(48000);engine->noteOn(60,1,1);Buffer out;for(int i=0;i<6000;++i){if(i==2048&&edit){if(modern)engine->parameters.route=5;else{engine->parameters.rack.order=23;engine->parameters.rack.modules[0].type=6;}}if(i%128==0)engine->beginBlock();out.push_back(engine->next());}return out;};for(bool modern:{false,true})check(diff(inactiveEdit(modern,false),inactiveEdit(modern,true))<1e-8,"inactive motor automation changed audio");
 // Stress: four different modules, sixteen sustained voices, live type/order changes.
 p=anks::Parameters{};p.useSample=true;p.loop=true;p.rack.enabled=true;for(int i=0;i<4;++i){p.rack.modules[size_t(i)].type=i+1;p.rack.modules[size_t(i)].mix=.5;}e->parameters=p;e->sample=&s;e->prepare(48000);for(int i=0;i<16;++i)e->noteOn(40+i,1,.3);auto start=std::chrono::steady_clock::now();for(int i=0;i<48000;++i){if(i%128==0)e->beginBlock();auto x=e->next();check(std::isfinite(x),"stress finite");}double seconds=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();for(int i=0;i<24;++i){e->parameters.rack.order=i;e->parameters.rack.modules[3].type=i%7;e->beginBlock();for(int k=0;k<512;++k)check(std::isfinite(e->next()),"topology transition finite");}
 std::cout<<"PASS: six module types; empty bypass; block invariance; both bodies; duplicate instances; order; four-band EQ; live-input grains; history reset; live topology. 16 voices / 4 modules / 1 s audio: "<<seconds<<" s on test host.\n";
}
