#include "Engine.h"
#include <iostream>
#include <stdexcept>
#include <memory>
#include <vector>
#include <complex>
#include <chrono>
void check(bool ok,const char*msg){if(!ok)throw std::runtime_error(msg);}
double amplitude(const std::vector<double>&x,double hz,int start=4096){std::complex<double>sum{};for(size_t i=size_t(start);i<x.size();++i)sum+=x[i]*std::polar(1.,-2*anks::pi*hz*double(i)/48000);return std::abs(sum)/(x.size()-size_t(start));}
int main(){
 anks::FrequencyShift shift;shift.prepare(48000);std::vector<double>x;
 for(int i=0;i<48000;++i)x.push_back(shift.next(.2*std::cos(2*anks::pi*1000*i/48000),200));
 std::cout<<"Shift +200 Hz: wanted "<<amplitude(x,1200)<<" mirror "<<amplitude(x,800)<<'\n';
 check(amplitude(x,1200)>.09&&amplitude(x,800)<.002,"frequency shift is not single sideband");
 shift.reset();x.clear();for(int i=0;i<48000;++i)x.push_back(shift.next(.2*std::cos(2*anks::pi*1000*i/48000),-200));check(amplitude(x,800)>.09&&amplitude(x,1200)<.002,"negative frequency shift");
 anks::SpectralHold freeze;freeze.prepare();double error=0,energy=0;std::vector<double>input(48000);for(int i=0;i<48000;++i){input[size_t(i)]=.2*std::sin(2*anks::pi*440*i/48000);double y=freeze.next(input[size_t(i)],false);if(i>4096)error+=std::abs(y-input[size_t(i-1024)]);}check(error<1e-7,"live spectral reconstruction");
 freeze.reset();for(int i=0;i<48000;++i){double y=freeze.next(i<12000?.2*std::sin(2*anks::pi*440*i/48000):0,i>=8000);check(std::isfinite(y)&&std::abs(y)<.5,"freeze unsafe");if(i>24000)energy+=y*y;}check(energy>10,"spectral hold lost input after silence");
 for(int i=0;i<12000;++i)check(std::isfinite(freeze.next(0,false)),"freeze release unsafe");
 double rest=0;for(int i=0;i<2048;++i)rest+=std::abs(freeze.next(0,false));check(rest<1e-8,"freeze does not release");freeze.reset();for(int i=0;i<3000;++i)check(freeze.next(0,true)==0,"freeze history leaked");
 anks::SemiModular semi;semi.prepare(48000);anks::RackParameters p;p.smDrone=true;p.smAmount={0,0};p.smCutoff=800;p.smResonance=.1;semi.configure(p);double low=0,high=0;for(int i=0;i<24000;++i){double y=semi.next(.1*std::sin(2*anks::pi*100*i/48000),p);if(i>4096)low+=y*y;}semi.reset();semi.configure(p);for(int i=0;i<24000;++i){double y=semi.next(.1*std::sin(2*anks::pi*8000*i/48000),p);if(i>4096)high+=y*y;}check(low>1&&high<low*.001,"semi VCF response");
 semi.reset();semi.configure(p);semi.setGate(true);for(int i=0;i<20000;++i)check(semi.next(0,p)==0,"semi generated an unsolicited oscillator");
 p.smDrone=false;p.smRelease=.03;semi.configure(p);semi.setGate(false);for(int i=0;i<12000;++i)semi.next(.1,p);check(std::abs(semi.next(.1,p))<1e-6,"semi envelope release");
 // Four-module reordering remains a serial transformation; block size must not change output.
 auto render=[](int block,int order){auto e=std::make_unique<anks::Engine>();e->parameters.rack.enabled=true;e->parameters.waveform=1;e->parameters.rack.order=order;int types[]={1,4,7,9};for(int j=0;j<4;++j)e->parameters.rack.modules[size_t(j)].type=types[j];e->prepare(48000);e->noteOn(60,1,.2);std::vector<double>out;for(int i=0;i<12000;++i){if(i%block==0)e->beginBlock();auto y=e->nextStereo();check(std::isfinite(y.left)&&std::isfinite(y.right),"creative rack nonfinite");out.push_back(y.left);}return out;};auto a=render(32,0),b=render(512,0),c=render(128,23);double different=0;for(size_t i=0;i<a.size();++i){check(std::abs(a[i]-b[i])<1e-8,"creative callback invariance");different+=std::abs(a[i]-c[i]);}check(different>.01,"creative order inaudible");
 std::cout<<"PASS: signed single-sideband shift, spectral reconstruction/hold/release/reset, VCF response, no internal audio source, envelope release, rack order and callback invariance.\n";
}
