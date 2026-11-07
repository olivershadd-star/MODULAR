#include "Engine.h"
#include <memory>
#include <iostream>
#include <vector>
#include <stdexcept>
using Frames=std::vector<anks::Stereo>;
void check(bool ok,const char*s){if(!ok)throw std::runtime_error(s);}
anks::Sample impact(){anks::Sample s;s.rate=48000;s.mono.resize(4800);uint32_t r=123;for(size_t i=0;i<384;++i){r^=r<<13;r^=r>>17;r^=r<<5;s.mono[i]=float(.35*(double(r)/4294967295.*2-1)*std::exp(-double(i)/110));}return s;}
Frames play(anks::Parameters p,const anks::Sample&s,int gate=3600,int count=96000,int block=128){auto e=std::make_unique<anks::Engine>();e->parameters=p;e->sample=&s;e->prepare(48000);e->noteOn(60,1,1);Frames out;out.reserve(size_t(count));for(int i=0;i<count;++i){if(i%block==0){e->parameters.rack.ppq=i/24000.;e->beginBlock();}if(i==gate)e->noteOff(60,1);auto x=e->nextStereo();check(std::isfinite(x.left)&&std::isfinite(x.right)&&std::max(std::abs(x.left),std::abs(x.right))<=.98,"unsafe stereo sample");out.push_back(x);}return out;}
double energy(const Frames&f,int a,int b){double e=0;for(int i=a;i<b;++i)e+=f[size_t(i)].left*f[size_t(i)].left+f[size_t(i)].right*f[size_t(i)].right;return e;}
double diff(const Frames&a,const Frames&b){double d=0;for(size_t i=0;i<a.size();++i)d+=std::abs(a[i].left-b[i].left)+std::abs(a[i].right-b[i].right);return d;}
int main(){auto s=impact();anks::Parameters p;p.useSample=true;p.release=.01;p.rack.enabled=true;auto&m=p.rack.modules[0];m.type=5;m.mix=1;
 auto delay=play(p,s);check(energy(delay,0,11999)<1e-12,"wet delay leaks undelayed source");double first=energy(delay,12000,13000),second=energy(delay,24000,25000);check(first>.01,"first echo lost after MIDI note-off");check(second/first>.02&&second/first<.15,"delay feedback is not a decaying repetition");
 m.time=.5;auto later=play(p,s);check(energy(later,0,23999)<1e-12&&energy(later,24000,25000)>.01,"delay time inactive");m.time=.25;m.feedback=0;auto single=play(p,s);check(energy(single,23500,30000)<1e-10,"zero feedback still repeats");
 m=anks::RackParameters{};m.type=4;m.mix=1;auto grains=play(p,s,960);check(energy(grains,0,960)>.001,"granular onset inaudible with default controls");check(energy(grains,2000,8000)>.001,"grains cut off by short MIDI gate");check(diff(grains,play([&]{auto q=p;q.rack.modules[0].pitch=12;return q;}(),s,960))>1,"grain pitch inaudible");check(diff(grains,play([&]{auto q=p;q.rack.modules[0].grainSize=.02;return q;}(),s,960))>1,"grain size inaudible");check(diff(grains,play([&]{auto q=p;q.rack.modules[0].density=60;return q;}(),s,960))>1,"grain density inaudible");
 // Full-wet granular output starts after a real capture, not an injected dry safety path.
 check(energy(grains,0,200)==0,"granular dry path was injected");
 m=anks::RackParameters{};m.type=6;m.mix=1;auto chorus=play(p,s,960);double width=0;for(auto x:chorus)width+=std::abs(x.left-x.right);check(width>.5,"chorus not stereo");check(energy(chorus,0,100)==0,"wet chorus contains dry input");check(diff(chorus,play([&]{auto q=p;q.rack.modules[0].depth=1;return q;}(),s,960))>1,"chorus depth inaudible");
 // Long sources: modulation speed changes the wet signal, including through following filters.
 auto tone=s;tone.mono.resize(48000);for(size_t i=0;i<tone.mono.size();++i)tone.mono[i]=float(.15*std::sin(2*anks::pi*440*i/48000.));auto c1=play(p,tone,48000);m.rate=4;check(diff(c1,play(p,tone,48000))>10,"chorus speed inactive");p.rack.modules[1].type=2;p.rack.modules[1].mix=1;auto filtered=play(p,tone,48000);width=0;for(auto x:filtered)width+=std::abs(x.left-x.right);check(width>1,"later filter collapsed stereo");
 // One frame and host-like chunks must yield the same musical result.
 check(diff(play(p,s,3600,24000,1),play(p,s,3600,24000,512))<1e-6,"effect output depends on host block size");
 p.rack.modules[1].type=0;m.type=3;m.drive=30;m.distoMode=0;auto soft=play(p,tone);m.distoMode=1;auto hard=play(p,tone);m.distoMode=2;auto fold=play(p,tone);check(diff(soft,hard)>1&&diff(hard,fold)>1,"distortion modes identical");
 std::cout<<"PASS: 75 ms clip gate / full-wet echo after note-off; echo timing and decay; 20 ms gate / granular startup, tail and controls; no hidden dry rescue; stereo multi-voice chorus, depth and speed; stereo through later blocks; callback invariance; three distinct distortion modes.\n";
}
