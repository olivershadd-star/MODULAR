#include "Engine.h"
#include <set>
#include <iostream>
#include <memory>
#include <stdexcept>
void check(bool v,const char*m){if(!v)throw std::runtime_error(m);}
int main(){
 std::set<std::array<int,6>> unique;for(auto order:anks::rackOrders()){auto sorted=order;std::sort(sorted.begin(),sorted.end());check(sorted==std::array<int,6>{{0,1,2,3,4,5}},"invalid permutation");unique.insert(order);}check(unique.size()==720,"missing six-slot order");std::array<int,4> old{{0,1,2,3}};int index=0;do{auto order=anks::rackOrders()[size_t(index++)];for(int i=0;i<4;++i)check(order[size_t(i)]==old[size_t(i)],"old order changed");check(order[4]==4&&order[5]==5,"old order E/F defaults");}while(std::next_permutation(old.begin(),old.end()));
 // New processors must transform audio, remain finite, preserve channel separation and reset exactly.
 for(int type=10;type<=13;++type)for(double rate:{44100.,48000.,96000.}){
  auto m=std::make_unique<anks::StereoRackModule>();m->prepare(rate);anks::RackParameters p;p.type=type;p.mix=1;p.fxFrequency=330;p.fxFeedback=.7;p.depth=.9;m->configure(p,220,120,0,true);double change=0,energy=0;
  for(int i=0;i<int(rate*.3);++i){double x=.1*std::sin(2*anks::pi*220*i/rate);auto y=m->next({x,0});check(std::isfinite(y.left)&&std::abs(y.left)<=16,"new FX unsafe");check(y.right==0,"new FX channel leak");change+=std::abs(x-y.left);energy+=y.left*y.left;}check(energy>.01&&change>1,"new FX no audible processing");
  m->reset();m->configure(p,220,120,0,true);for(int i=0;i<1024;++i){auto y=m->next({0,0});check(y.left==0&&y.right==0,"new FX reset leaks");}
 }
 auto engine=std::make_unique<anks::Engine>();auto&p=engine->parameters;p.rack.enabled=true;p.fm.enabled=true;p.fm.ratio=1.414;p.fm.index=3;const int types[]={1,4,9,10,12,13};for(size_t i=0;i<6;++i){p.rack.modules[i].type=types[i];p.rack.modules[i].mix=.4;}engine->prepare(48000);for(int i=0;i<8;++i)engine->noteOn(48+i,1,.4);
 for(int block=0;block<720;++block){p.rack.order=block;p.rack.modules[5].fxFrequency=100+block;engine->beginBlock();for(int i=0;i<32;++i){auto x=engine->nextStereo();check(std::isfinite(x.left)&&std::abs(x.left)<=.98,"six slot topology unsafe");}}
 std::cout<<"PASS: 720 unique orders, 24 legacy orders, four FX at three rates, stereo/reset and FM six-slot live topology.\n";
}
