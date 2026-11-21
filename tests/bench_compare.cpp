#define anks reference
#ifndef ANKS_REFERENCE_ENGINE
#error "Configure ANKS_REFERENCE_SOURCE to the reference checkout"
#endif
#include ANKS_REFERENCE_ENGINE
#undef anks
#include "Engine.h"
#include <chrono>
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
#include <array>
#include <vector>
#include <cstring>
#if defined(__SSE__) || defined(_M_X64)
#include <xmmintrin.h>
#endif
using Audio=std::array<std::array<double,2>,256>;
struct Scenario{const char*name;std::array<int,4>types;bool fm=false,topology=false;};
template<class E>void setup(E&e,const Scenario&s){auto&p=e.parameters;p.rack.enabled=true;p.waveform=1;p.fm.enabled=s.fm;p.fm.feedback=.3;p.fm.modSustain=.5;for(int j=0;j<4;++j){auto&m=p.rack.modules[size_t(j)];m.type=s.types[size_t(j)];m.density=64;m.grainSize=.25;m.mix=.75;m.freezeHold=true;}e.prepare(48000);}
template<class E>void callback(E&e,const Scenario&s,int offset,int block,Audio&out){auto&p=e.parameters;int retrigger=24000/block*block;if(offset==0||offset==retrigger)for(int v=0;v<16;++v)e.noteOn(36+v,1,.015);if(offset==retrigger-block)for(int v=0;v<16;++v)e.noteOff(36+v,1);p.rack.ppq=2.*offset/48000;p.rack.modules[0].body[0].brightness=.5+.35*std::sin(offset/48000.*11);p.rack.modules[1].grainSize=.14+.10*std::sin(offset/48000.*7);if(s.topology){p.rack.order=offset>19200&&offset<33600?23:0;p.rack.modules[3].type=offset>19200&&offset<33600?8:9;}e.beginBlock();for(int i=0;i<std::min(block,48000-offset);++i){auto y=e.nextStereo();out[size_t(i)]={y.left,y.right};}}
double cpuMicros(){
#if defined(_WIN32)
 FILETIME creation,exit,kernel,user;if(!GetProcessTimes(GetCurrentProcess(),&creation,&exit,&kernel,&user))return 0;ULARGE_INTEGER k,u;k.LowPart=kernel.dwLowDateTime;k.HighPart=kernel.dwHighDateTime;u.LowPart=user.dwLowDateTime;u.HighPart=user.dwHighDateTime;return double(k.QuadPart+u.QuadPart)/10.;
#else
 return double(std::clock())*1e6/CLOCKS_PER_SEC;
#endif
}
int main(){
#if defined(__SSE__) || defined(_M_X64)
_mm_setcsr(_mm_getcsr()|0x8040);
#endif
Scenario cases[]={{"empty",{0,0,0,0}},{"res_two_granular",{1,4,4,0}},{"res_two_granular_semi",{1,4,4,9}},{"four_resonators",{1,1,1,1}},{"fm_res_gran_freeze_semi",{1,4,8,9},true},{"live_topology",{1,4,4,9},false,true}};std::cout<<"scenario,block,before_cpu_us,after_cpu_us,before_p99_us,after_p99_us,before_note_max_us,after_note_max_us\n"<<std::fixed<<std::setprecision(3);for(auto&s:cases)for(int block:{64,128,256}){auto a=std::make_unique<reference::Engine>();auto b=std::make_unique<anks::Engine>();setup(*a,s);setup(*b,s);Audio x{},y{};double cpu[2]{},note[2]{};std::vector<double>t[2];for(int i=0;i<48000;i+=block){auto run=[&](int who){auto start=std::chrono::steady_clock::now();double c=cpuMicros();if(who==0)callback(*a,s,i,block,x);else callback(*b,s,i,block,y);cpu[who]+=cpuMicros()-c;double us=std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-start).count();t[who].push_back(us);if(i==0||i==24000/block*block)note[who]=std::max(note[who],us);};if((i/block)%2){run(1);run(0);}else{run(0);run(1);}for(int sample=0;sample<std::min(block,48000-i);++sample)if(!std::isfinite(x[size_t(sample)][0])||!std::isfinite(x[size_t(sample)][1])||!std::isfinite(y[size_t(sample)][0])||!std::isfinite(y[size_t(sample)][1]))return 3;if(std::memcmp(x.data(),y.data(),size_t(std::min(block,48000-i))*sizeof(x[0]))!=0){std::cerr<<"audio mismatch "<<s.name<<'\n';return 1;}}for(auto&v:t)std::sort(v.begin(),v.end());auto n=t[0].size();std::cout<<s.name<<','<<block<<','<<cpu[0]/n<<','<<cpu[1]/n<<','<<t[0][size_t(n*.99)]<<','<<t[1][size_t(n*.99)]<<','<<note[0]<<','<<note[1]<<std::endl;}}
