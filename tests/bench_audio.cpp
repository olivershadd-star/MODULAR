#include "Engine.h"
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <ctime>
#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif
#include <iostream>
#include <iomanip>
#include <memory>
#include <string>
#include <vector>
#if defined(__SSE__ ) || defined(_M_X64)
#include <xmmintrin.h>
#endif
double cpuMicros(){
#if defined(_WIN32)
 FILETIME creation,exit,kernel,user;if(!GetProcessTimes(GetCurrentProcess(),&creation,&exit,&kernel,&user))return 0;ULARGE_INTEGER k,u;k.LowPart=kernel.dwLowDateTime;k.HighPart=kernel.dwHighDateTime;u.LowPart=user.dwLowDateTime;u.HighPart=user.dwHighDateTime;return double(k.QuadPart+u.QuadPart)/10.;
#else
 return double(std::clock())*1e6/CLOCKS_PER_SEC;
#endif
}
int main(int argc,char**argv){
#if defined(__SSE__) || defined(_M_X64)
 _mm_setcsr(_mm_getcsr()|0x8040);
#endif
 bool full=false;std::string selected;FILE*dump=nullptr;for(int a=1;a<argc;++a){if(std::string(argv[a])=="--full")full=true;else if(std::string(argv[a])=="--scenario"&&a+1<argc)selected=argv[++a];else if(std::string(argv[a])=="--dump"&&a+1<argc){dump=std::fopen(argv[++a],"wb");if(!dump)return 2;}else return 2;}
 struct Scenario{const char*name;std::array<int,4>types;bool fm=false,topology=false,legacy=false,bypass=false;};
 const Scenario cases[]={{"empty",{0,0,0,0}},{"granular_semi_OFF",{1,4,4,9},false,false,false,true},{"resonator_two_granular",{1,4,4,0}},{"resonator_two_granular_semi",{1,4,4,9}},{"four_resonators",{1,1,1,1}},{"fm_res_gran_freeze_semi",{1,4,8,9},true},{"live_topology",{1,4,4,9},false,true},{"legacy_toggle",{1,4,4,9},false,false,true}};
 if(!selected.empty()){bool found=false;for(const auto&scenario:cases)found=found||selected==scenario.name;if(!found)return 2;}
 const std::vector<double>rates=full?std::vector<double>{44100,48000,96000}:std::vector<double>{48000};
 const std::vector<int>blocks=full?std::vector<int>{64,128,256}:std::vector<int>{128};
 std::cout<<"scenario,sample_rate,block,chord_notes,active_voices_peak,mean_us,p95_us,p99_us,max_us,deadline_us,overruns,note_on_max_us,cpu_mean_us\n"<<std::fixed<<std::setprecision(3);
 for(auto&scenario:cases)for(double sr:rates)for(int block:blocks)for(int count:{1,8,16}){
  if(!selected.empty()&&selected!=scenario.name)continue;
  auto e=std::make_unique<anks::Engine>();auto&p=e->parameters;p.rack.enabled=true;p.waveform=1;p.fm.enabled=scenario.fm;p.fm.feedback=.3;p.fm.modSustain=.5;
  for(size_t i=0;i<4;++i){auto&m=p.rack.modules[i];m.type=scenario.types[i];m.on=!scenario.bypass||i==0;m.density=64;m.grainSize=.25;m.mix=.75;m.freezeHold=true;}
  e->prepare(sr);std::vector<double>times;std::vector<anks::Stereo>audio(size_t(block),anks::Stereo{});double noteMax=0,total=0,cpuTotal=0;int overruns=0,activePeak=0;
  const double deadline=1e6*block/sr;const int frames=int(sr),retrigger=int(sr*.5)/block*block;
  for(int offset=0;offset<frames;offset+=block){auto start=std::chrono::steady_clock::now();double cpuStart=cpuMicros();bool note=offset==0||offset==retrigger;
   if(note)for(int v=0;v<count;++v)e->noteOn(36+v,1,.015);
   if(offset==retrigger-block)for(int v=0;v<count;++v)e->noteOff(36+v,1);
   p.rack.ppq=2.*offset/sr;p.rack.modules[0].body[0].brightness=.5+.35*std::sin(offset/sr*11);p.rack.modules[1].grainSize=.14+.10*std::sin(offset/sr*7);
   if(scenario.topology){p.rack.order=offset>sr*.4&&offset<sr*.7?23:0;p.rack.modules[3].type=offset>sr*.4&&offset<sr*.7?8:9;}
   if(scenario.legacy){p.rack.enabled=!(offset>sr*.3&&offset<sr*.8);p.body[0].brightness=.2+.6*std::abs(std::sin(offset/sr*9));}
   e->beginBlock();int n=std::min(block,frames-offset);for(int i=0;i<n;++i){audio[size_t(i)]=e->nextStereo();if(!std::isfinite(audio[size_t(i)].left)||!std::isfinite(audio[size_t(i)].right))return 3;}
   cpuTotal+=cpuMicros()-cpuStart;double us=std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-start).count();times.push_back(us);total+=us;if(us>deadline)++overruns;if(note)noteMax=std::max(noteMax,us);activePeak=std::max(activePeak,e->activeVoices());
   if(dump&&std::fwrite(audio.data(),sizeof(anks::Stereo),size_t(n),dump)!=size_t(n))return 4;
  }
  std::sort(times.begin(),times.end());auto percentile=[&](double p){return times[std::min(times.size()-1,size_t(p*times.size()))];};
  std::cout<<scenario.name<<','<<int(sr)<<','<<block<<','<<count<<','<<activePeak<<','<<total/times.size()<<','<<percentile(.95)<<','<<percentile(.99)<<','<<times.back()<<','<<deadline<<','<<overruns<<','<<noteMax<<','<<cpuTotal/times.size()<<'\n'<<std::flush;
 }
 if(dump)std::fclose(dump);
}
