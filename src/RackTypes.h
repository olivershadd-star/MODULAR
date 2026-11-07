#pragma once
#include <array>
#include <algorithm>
namespace anks {
struct Stereo {double left=0,right=0;double mono()const{return (left+right)*.5;}};

enum class ModuleType:int {Empty,Resonator,Filter,Saturation,Granular,Delay,Chorus};
struct RackBody {double decay=1.5,brightness=.8,position=.2,detune=0,level=0;bool on=true,odd=false;};
struct RackParameters {
 int type=0,filterMode=0,lfoShape=0,distoMode=0;bool on=true,lfoSync=false;
 double mix=.8,balance=.5,cutoff=6000,q=.707,drive=6,tone=6000,level=0;
 double time=.25,feedback=.3,rate=.5,depth=.3;
 double grainSize=.08,density=20,position=.3,scatter=.1,pitch=0;
 double lfoRate=.5,lfoDepth=0,lfoBeats=4;
 std::array<RackBody,2>body{};
 std::array<double,4>eqFrequency{{150,650,2500,8000}},eqGain{},eqQ{{.707,.707,.707,.707}};
};
// The order is one permutation parameter: never four independently automatable IDs.
inline const std::array<std::array<int,4>,24>& rackOrders(){
 static const auto orders=[] {std::array<std::array<int,4>,24>o{};std::array<int,4>a{{0,1,2,3}};size_t n=0;do{o[n++]=a;}while(std::next_permutation(a.begin(),a.end()));return o;}();return orders;
}
struct RackConfig {bool enabled=false,clockRunning=true;int order=0;double bpm=120,ppq=0;std::array<RackParameters,4>modules{};};
}
