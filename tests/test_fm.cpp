#include "Engine.h"
#include <memory>
#include <vector>
#include <complex>
#include <iostream>
#include <stdexcept>
#include <chrono>
void check(bool ok,const char*msg){if(!ok)throw std::runtime_error(msg);}
double amplitude(const std::vector<double>&x,double hz){std::complex<double>s{};for(size_t i=4096;i<x.size();++i)s+=x[i]*std::polar(1.,-2*anks::pi*hz*double(i)/48000);return std::abs(s)/double(x.size()-4096);}
std::vector<double>render(anks::FmParameters p,double velocity=1,double note=69){anks::FmSource f;f.prepare(48000);f.configure(anks::midiHz(note),p,0,.1,velocity,true);std::vector<double>out;for(int i=0;i<48000;++i){double v=f.next(true);check(std::isfinite(v)&&std::abs(v)<.4,"FM bounds");out.push_back(v);}return out;}
double difference(const std::vector<double>&a,const std::vector<double>&b){double d=0;for(size_t i=0;i<a.size();++i)d+=std::abs(a[i]-b[i]);return d;}
int main(){
 anks::FmParameters p;p.modAttack=.001;p.modSustain=1;p.index=0;p.velocity=0;
 auto sine=render(p);check(amplitude(sine,440)>.12&&amplitude(sine,1320)<.001,"zero index must be sine");
 p.index=2;p.ratio=2;auto fm=render(p);check(amplitude(fm,1320)>.02&&difference(sine,fm)>100,"FM sidebands missing");
 p.ratio=1.414;check(difference(fm,render(p))>100,"ratio inactive");p.feedback=.7;check(difference(render(p),[&]{auto q=p;q.feedback=0;return render(q);}())>100,"feedback inactive");
 p.feedback=0;p.velocity=1;check(difference(render(p,.1),render(p,1))>100,"velocity brightness inactive");
 p.velocity=0;p.modSustain=0;p.modDecay=.03;auto decaying=render(p);check(amplitude(decaying,1320)<.002,"modulation envelope does not decay");
 anks::FmSource f;f.prepare(48000);f.configure(440,p,0,.02,1,true);for(int i=0;i<12000;++i)f.next(true);for(int i=0;i<10000;++i)f.next(false);check(std::abs(f.next(false))<1e-8,"volume release");f.reset();f.configure(440,p,0,.1,1,true);double first=f.next(true);f.reset();f.configure(440,p,0,.1,1,true);check(first==f.next(true),"FM reset not deterministic");
 for(double sr:{8000.,44100.,48000.,96000.,192000.}){p.index=12;p.feedback=.95;p.ratio=16;f.prepare(sr);f.configure(anks::midiHz(120),p,.01,.1,.9,true);for(int i=0;i<4096;++i)check(std::isfinite(f.next(i<2048)),"extreme FM nonfinite");}
 auto chain=[](int block,bool sourceSample,int order){auto e=std::make_unique<anks::Engine>();auto&p=e->parameters;p.fm.enabled=true;p.fm.modSustain=.5;p.rack.enabled=true;p.rack.order=order;p.rack.modules[0].type=4;p.rack.modules[1].type=1;p.rack.modules[2].type=9;anks::Sample sample;sample.mono.assign(48000,.4f);e->sample=&sample;p.useSample=sourceSample;e->prepare(48000);e->noteOn(60,1,.3);std::vector<double>out;for(int i=0;i<16000;++i){if(i%block==0)e->beginBlock();if(i==12000)e->noteOff(60,1);auto y=e->nextStereo();check(std::isfinite(y.left)&&std::abs(y.left)<=.98,"FM chain unsafe");out.push_back(y.left);}return out;};auto a=chain(32,false,0),b=chain(512,false,0),c=chain(128,true,0);check(difference(a,b)<1e-7,"FM callback invariance");check(difference(a,c)<1e-7,"FM layered with WAV");check(difference(a,chain(128,false,23))>.01,"FM rack order inactive");
 std::cout<<"PASS: pure carrier, FM sidebands, ratio/feedback/velocity, modulation decay, release/reset, extreme rates, single source, callback invariance and rack order.\n";
}
