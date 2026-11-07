#include "Engine.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
#include <chrono>
void check(bool ok,const char*m){if(!ok)throw std::runtime_error(m);}
anks::Sample sample(){anks::Sample s;s.rate=48000;s.mono.resize(4800);for(size_t i=0;i<s.mono.size();++i)s.mono[i]=float(0.18*std::sin(2*anks::pi*220*double(i)/48000)+0.09*std::sin(2*anks::pi*1379*double(i)/48000));return s;}
std::vector<double>render(anks::Parameters p,const anks::Sample&s,int count=12000){auto e=std::make_unique<anks::Engine>();e->parameters=p;e->sample=&s;e->prepare(48000);e->noteOn(60,1,1);std::vector<double>out;out.reserve(size_t(count));for(int i=0;i<count;++i){if(i%128==0)e->beginBlock();double v=e->next();check(std::isfinite(v)&&std::abs(v)<=0.98,"nonfinite/guard");out.push_back(v);}return out;}
double distance(const std::vector<double>&a,const std::vector<double>&b){double sum=0;for(size_t i=0;i<a.size();++i)sum+=std::abs(a[i]-b[i]);return sum;}
int main(){
 int cases=0;
 for(double sr:{44100.,48000.,96000.,192000.})for(int note:{24,36,48,60,72,84,96})for(double bright:{0.05,0.3,0.8,1.0})for(bool odd:{false,true}){
  auto r=std::make_unique<anks::Resonator>();r->prepare(sr);anks::BodyParameters b;b.decay=0.8;b.brightness=bright;b.odd=odd;r->tune(anks::midiHz(note),b);
  double w=2*anks::pi*anks::midiHz(note)/sr,d=r->delaySamples(),n=std::floor(d),f=d-n,q=1-bright;double phase=n*w+std::atan2(f*std::sin(w),1-f+f*std::cos(w))+std::atan2(q*std::sin(w),1-q*std::cos(w));check(std::abs(phase-(odd?anks::pi:2*anks::pi))<1e-8,"phase tuning");
  double early=0,late=0;for(int i=0;i<int(sr*2);++i){double x=r->next(i==0?1:0);check(std::isfinite(x)&&std::abs(x)<3,"impulse bounds");if(i<sr/2)early+=x*x;if(i>sr*1.5)late+=x*x;}check(early>0&&late<early*0.01,"decay");++cases;
 }
 auto s=sample();anks::Parameters p;p.useSample=true;p.bodiesOn=false;
 auto dry=render(p,s);for(size_t i=48;i<s.mono.size();++i)check(std::abs(dry[i]-s.mono[i])<1e-7,"dry unity gain");
 p.outputDb=6;auto louder=render(p,s);double expected=anks::dbGain(6);for(size_t i=48;i<s.mono.size();++i)check(std::abs(louder[i]-dry[i]*expected)<1e-6,"output dB gain");
 p=anks::Parameters{};p.useSample=true;p.bodyMix=1;p.balance=0;p.body[1].brightness=0.3;auto a=render(p,s);p.body[1].decay=7;p.body[1].position=0.4;check(distance(a,render(p,s))<1e-8,"B leaks into A endpoint");p.balance=1;auto b=render(p,s);p.body[0].decay=0.1;check(distance(b,render(p,s))<1e-8,"A leaks into B endpoint");
 p.body[0]=p.body[1];p.balance=0.5;auto both=render(p,s);p.body[1].enabled=false;check(distance(both,render(p,s))<1e-7,"identical bodies double gain");
 p=anks::Parameters{};p.useSample=true;p.filterOn=true;p.driveOn=true;p.cutoff=700;p.driveDb=15;p.body[1].detune=13;p.body[1].position=0.31;auto order0=render(p,s);p.route=2;check(distance(order0,render(p,s))>0.01,"routing has no audible effect");for(int route=0;route<6;++route){p.route=route;render(p,s);}
 // MIDI: channel independence, sustain, stolen notes and clip guard under sustained excitation.
 auto e=std::make_unique<anks::Engine>();e->parameters.useSample=true;e->parameters.loop=true;e->parameters.release=0.03;e->sample=&s;e->prepare(48000);
 auto advance=[&](int count){for(int i=0;i<count;++i){if(i%128==0)e->beginBlock();double x=e->next();check(std::isfinite(x)&&std::abs(x)<=0.98,"stress finite");}};
 e->noteOn(60,1,1);e->setSustain(1,true);e->noteOff(60,1);advance(4800);check(e->activeVoices()==1,"sustain hold");e->setSustain(1,false);advance(2000);check(e->activeVoices()==0,"sustain release");
 e->noteOn(60,1,1);e->noteOn(60,2,1);e->noteOff(60,1);advance(2000);check(e->activeVoices()==1,"MIDI channel isolation");e->control(2,120,0);check(e->activeVoices()==0,"all sound off");
 e->setSustain(1,true);for(int i=0;i<40;++i){e->noteOn(36+i%24,1,1);e->noteOff(36+i%24,1);}check(e->activeVoices()==16,"stealing");e->setSustain(1,false);advance(2000);check(e->activeVoices()==0,"stolen notes stuck");
 e->noteOn(60,1,1);e->pitchBend(1,2);advance(1000);for(int i=0;i<6;++i){e->parameters.route=i;e->parameters.body[0].brightness=0.1+0.1*i;advance(1500);}check(e->activeVoices()==1,"live routing killed voice");
 e->control(1,11,0);double silence=0;for(int i=0;i<100;++i)silence+=std::abs(e->next());check(silence==0,"expression mute");e->panic();
 e->parameters.inputDb=24;e->parameters.outputDb=24;e->parameters.driveOn=true;e->parameters.driveDb=24;e->parameters.body[0].decay=8;e->parameters.body[1].decay=8;e->beginBlock();for(int i=0;i<16;++i)e->noteOn(36+i,1,1);
 auto start=std::chrono::steady_clock::now();bool clipped=false;for(int i=0;i<48000;++i){if(i%128==0){clipped|=e->clipped;e->beginBlock();}auto x=e->next();check(std::isfinite(x)&&std::abs(x)<=0.98,"high gain guard");}check(clipped||e->clipped,"clip meter missing");
 std::cout<<"PASS: "<<cases<<" phase/decay cases; dry unity; dB gain; A/B isolation; identical-body gain; 6 routes; sustain/channels/stealing/live controls; 16-voice sustained stress.\n";
 std::cout<<"16-voice render 1 second: "<<std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count()<<" s on test host (not user PC).\n";
}
