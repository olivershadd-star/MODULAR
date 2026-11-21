#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>
#include "RackTypes.h"
#include "FmSource.h"
namespace anks {
constexpr double pi=3.14159265358979323846;
inline double midiHz(double n){return 440*std::exp2((n-69)/12);}
inline double dbGain(double d){return std::pow(10.0,d/20);}
struct Sample{std::vector<float>mono;double rate=48000;};
struct BodyParameters{
 double decay=1.5,brightness=0.8,position=0.2,detune=0,levelDb=0;
 bool enabled=true,odd=false;
 bool operator==(const BodyParameters&p)const{return decay==p.decay&&brightness==p.brightness&&position==p.position&&detune==p.detune&&levelDb==p.levelDb&&enabled==p.enabled&&odd==p.odd;}
};
struct Parameters{
 std::array<BodyParameters,2> body{};
 double inputDb=0,outputDb=0,bodyMix=0.8,balance=0.5,cutoff=6000,driveDb=6;
 double attack=0,release=0.3,root=60;
 double preparedTail=-1; // Derived by Engine::snapshot, never a saved parameter.
 bool filterOn=false,driveOn=false,bodiesOn=true,useSample=false,loop=false;
 int route=0;int waveform=0;RackConfig rack;FmParameters fm;
};
// Orders refer to real signal flow: filter, saturation, double resonator.
inline constexpr std::array<std::array<int,3>,6> routes{{{{0,1,2}},{{0,2,1}},{{1,0,2}},{{1,2,0}},{{2,0,1}},{{2,1,0}}}};
class Resonator{
public:
 static constexpr int capacity=16384;
 void prepare(double sr){rate=sr;slew=1-std::exp(-1/(rate*0.015));reset();}
 void reset(){valid=0;write=0;low=0;first=true;}
 void tune(double hz,const BodyParameters&p){
  hz=std::clamp(hz,20.0,rate/8);targetBright=std::clamp(p.brightness,0.05,1.0);
  const double w=2*pi*hz/rate,q=1-targetBright;
  const double phase=std::atan2(q*std::sin(w),1-q*std::cos(w));
  double lo=1,hi=std::min(double(capacity-2),rate/hz+2);
  for(int i=0;i<36;++i){double d=(lo+hi)*0.5,n=std::floor(d),f=d-n;double lag=n*w+std::atan2(f*std::sin(w),1-f+f*std::cos(w))+phase;if(lag<(p.odd?pi:2*pi))lo=d;else hi=d;}
  targetDelay=(lo+hi)*0.5;targetFeedback=std::pow(0.001,(p.odd?0.5:1)/(hz*std::clamp(p.decay,0.05,8.0)));
  targetPosition=std::clamp(p.position,0.02,0.48);polarity=p.odd?-1:1;
  if(first){delay=targetDelay;bright=targetBright;feedback=targetFeedback;position=targetPosition;first=false;}
 }
 void adoptTuning(const Resonator&source){targetBright=source.targetBright;targetDelay=source.targetDelay;targetFeedback=source.targetFeedback;targetPosition=source.targetPosition;polarity=source.polarity;if(first){delay=targetDelay;bright=targetBright;feedback=targetFeedback;position=targetPosition;first=false;}}
 double next(double x){
  delay+=slew*(targetDelay-delay);bright+=slew*(targetBright-bright);feedback+=slew*(targetFeedback-feedback);position+=slew*(targetPosition-position);
  double y=read(delay);low=(1-bright)*low+bright*y;data[write]=x+polarity*feedback*low;
  if(valid<capacity)++valid;
  double out=y-read(std::max(1.0,position*delay));write=(write+1)%capacity;return out;
 }
 double delaySamples()const{return targetDelay;}
private:
 double read(double d)const{double p=double(write)-d;if(p<0)p+=capacity;int a=int(p),b=(a+1)%capacity;if(valid==capacity)return data[a]+(data[b]-data[a])*(p-a);double av=a<valid?data[a]:0,bv=b<valid?data[b]:0;return av+(bv-av)*(p-a);}
 std::array<double,capacity>data{};int write=0,valid=0;bool first=true;
 double rate=48000,slew=0.001,low=0,polarity=1,delay=100,targetDelay=100,bright=0.8,targetBright=0.8,feedback=0.9,targetFeedback=0.9,position=0.2,targetPosition=0.2;
};
}
#include "RackDsp.h"
namespace anks {
struct Voice{
 bool active=false,held=false;int note=-1,channel=1,origin=0;uint64_t serial=0;double envelope=0;
 void prepare(double sr){rate=sr;fm.prepare(sr);slew=1-std::exp(-1/(rate*0.01));for(auto&r:bodies)r.prepare(sr);for(auto&r:rackModules)r.prepare(sr);active=false;}
 void start(int n,int ch,double vel,uint64_t order,const Parameters&p,const Sample*s,double pitch){
  note=n;channel=ch;velocity=vel;serial=order;sample=s;held=active=true;envelope=0;age=0;cursor=0;filterState=0;
  random=uint32_t(n+1)*747796405u+uint32_t(ch);for(auto&r:bodies)r.reset();bend=pitch;params=p;legacyPending=p.rack.enabled;legacyInitial=p.body;legacyNote=note+bend;
  input=dbGain(p.inputDb);mix=p.bodyMix;balance=p.balance;filterWet=p.filterOn?1:0;driveWet=p.driveOn?1:0;bodyWet=p.bodiesOn?1:0;drive=dbGain(p.driveDb);
  for(size_t i=0;i<2;++i){level[i]=dbGain(p.body[i].levelDb);enabled[i]=p.body[i].enabled?1:0;}
  fm.reset();fm.configure(midiHz(note+bend),p.fm,p.attack,p.release,velocity,true);tailAge=0;targets();filterCoefficient=filterTarget;refresh();oscPhase=0;for(size_t i=0;i<4;++i){rackModules[i].reset();rackModules[i].configure(p.rack.modules[i],midiHz(note+bend),p.rack.clockRunning?p.rack.bpm:0,p.rack.ppq,true);}
 }
 void apply(const Parameters&p){
  bool dirty=!(params.body[0]==p.body[0])||!(params.body[1]==p.body[1])||params.rack.enabled!=p.rack.enabled;params=p;targets();if(dirty)refresh();if(p.fm.enabled)fm.configure(midiHz(note+bend),p.fm,p.attack,p.release,velocity);
  step=(sample?sample->rate/rate:1)*std::exp2((note+bend-params.root)/12);
  if(p.rack.enabled)for(size_t i=0;i<4;++i)rackModules[i].configure(p.rack.modules[i],midiHz(note+bend),p.rack.clockRunning?p.rack.bpm:0,p.rack.ppq);
 }
 void pitch(double semitones){bend=semitones;refresh();if(params.fm.enabled)fm.configure(midiHz(note+bend),params.fm,params.attack,params.release,velocity);if(params.rack.enabled)for(size_t i=0;i<4;++i)rackModules[i].configure(params.rack.modules[i],midiHz(note+bend),params.rack.clockRunning?params.rack.bpm:0,params.rack.ppq,false,false);}
 void release(){held=false;}
 Stereo nextStereo(){
  if(!active)return {};
  if(held)envelope=std::min(1.0,envelope+attackStep);else envelope*=releaseFactor;
  if(envelope<0.00001&&!held){if(!params.rack.enabled||++tailAge>uint64_t(rate*tailSeconds)){active=false;return {};}}
  double x=0;
  if(params.fm.enabled)x=fm.next(held);
  else if(params.useSample&&sample&&!sample->mono.empty()){
   size_t len=sample->mono.size();if(cursor<double(len)){size_t i=size_t(cursor),j=i+1<len?i+1:(params.loop?size_t(0):i);x=sample->mono[i]+(sample->mono[j]-sample->mono[i])*(cursor-double(i));cursor+=step;if(params.loop&&cursor>=double(len))cursor=std::fmod(cursor,double(len));}
  }else if(params.waveform>0){x=.25*(params.waveform==1?std::sin(2*pi*oscPhase):params.waveform==2?4*std::abs(oscPhase-.5)-1:params.waveform==3?2*oscPhase-1:(oscPhase<.5?1:-1));oscPhase+=oscIncrement;oscPhase-=std::floor(oscPhase);}
  else{random^=random<<13;random^=random>>17;random^=random<<5;if(age<uint64_t(rate*0.008))x=(double(random)/4294967295.0*2-1)*std::exp(-double(age)/(rate*0.002))*0.5;}
  ++age;
  input+=slew*(inputTarget-input);mix+=slew*(params.bodyMix-mix);balance+=slew*(params.balance-balance);drive+=slew*(driveTarget-drive);
  filterWet+=slew*((params.filterOn?1.0:0.0)-filterWet);driveWet+=slew*((params.driveOn?1.0:0.0)-driveWet);bodyWet+=slew*((params.bodiesOn?1.0:0.0)-bodyWet);
  filterCoefficient+=slew*(filterTarget-filterCoefficient);
  x*=input*velocity;
  if(params.rack.enabled){double exciterEnvelope=params.fm.enabled?1.:(envelope>=.00001?envelope:0);Stereo signal{x*exciterEnvelope,x*exciterEnvelope};for(int id:rackOrders()[size_t(std::clamp(params.rack.order,0,23))]){rackModules[size_t(id)].setGate(held);signal=rackModules[size_t(id)].next(signal);}if(tailAge){double remaining=rate*tailSeconds-double(tailAge);double fade=std::clamp(remaining/(rate*.02),0.,1.);signal.left*=fade;signal.right*=fade;}return signal;}
  else for(int module:routes[size_t(std::clamp(params.route,0,5))]){
   if(module==0){filterState+=filterCoefficient*(x-filterState);x+=filterWet*(filterState-x);}
   else if(module==1){double y=std::tanh(x*drive);x+=driveWet*(y-x);}
   else{
    double a=bodies[0].next(x),b=bodies[1].next(x);
    for(size_t i=0;i<2;++i){level[i]+=slew*(levelTarget[i]-level[i]);enabled[i]+=slew*((params.body[i].enabled?1.0:0.0)-enabled[i]);}
    double wa=enabled[0]*(1-balance*enabled[1]);
    double wb=enabled[1]*(1-(1-balance)*enabled[0]);
    double activeWeight=enabled[0]+enabled[1]-enabled[0]*enabled[1];
    double y=wa*a*level[0]+wb*b*level[1]+(1-activeWeight)*x;
    x+=bodyWet*mix*(y-x);
   }
  }
  return {x*(params.fm.enabled?1.:envelope),x*(params.fm.enabled?1.:envelope)};
 }
private:
 void targets(){
  const bool changedRate=targetRate!=rate;
  tailSeconds=params.preparedTail>=0?params.preparedTail:rackTailSeconds(params.rack);
  if(params.inputDb!=cachedInput)inputTarget=dbGain(params.inputDb);
  if(params.driveDb!=cachedDrive)driveTarget=dbGain(params.driveDb);
  for(size_t i=0;i<2;++i)if(params.body[i].levelDb!=cachedLevel[i]){levelTarget[i]=dbGain(params.body[i].levelDb);cachedLevel[i]=params.body[i].levelDb;}
  if(changedRate||params.attack!=cachedAttack)attackStep=params.attack<=0?1:1/(rate*params.attack);
  if(changedRate||params.release!=cachedRelease)releaseFactor=std::exp(std::log(0.00001)/(rate*std::max(0.01,params.release)));
  if(changedRate||params.cutoff!=cachedCutoff)filterTarget=1-std::exp(-2*pi*std::clamp(params.cutoff,30.0,rate*0.45)/rate);
  targetRate=rate;cachedInput=params.inputDb;cachedDrive=params.driveDb;cachedAttack=params.attack;cachedRelease=params.release;cachedCutoff=params.cutoff;
 }

 void refresh(){oscIncrement=std::min(.45,midiHz(note+bend)/rate);
  if(!params.rack.enabled){if(legacyPending){for(size_t i=0;i<2;++i)bodies[i].tune(midiHz(legacyNote+legacyInitial[i].detune/100),legacyInitial[i]);legacyPending=false;}for(size_t i=0;i<2;++i)bodies[i].tune(midiHz(note+bend+params.body[i].detune/100),params.body[i]);}step=(sample?sample->rate/rate:1)*std::exp2((note+bend-params.root)/12);}
 std::array<BodyParameters,2>legacyInitial{};bool legacyPending=false;double legacyNote=60,oscIncrement=0;
 std::array<StereoRackModule,4>rackModules;double oscPhase=0,tailSeconds=0;uint64_t tailAge=0;
 double targetRate=-1,cachedInput=-1000,cachedDrive=-1000,cachedAttack=-1000,cachedRelease=-1000,cachedCutoff=-1000;std::array<double,2>cachedLevel{{-1000,-1000}};
 FmSource fm;Parameters params;const Sample*sample=nullptr;std::array<Resonator,2>bodies;
 double rate=48000,slew=0.001,velocity=0,step=1,cursor=0,bend=0,filterState=0;
 double input=1,mix=0.8,balance=0.5,drive=1,filterWet=0,driveWet=0,bodyWet=1;
 double inputTarget=1,driveTarget=1,filterCoefficient=0.5,filterTarget=0.5,attackStep=1,releaseFactor=0.99;
 std::array<double,2>level{{1,1}},levelTarget{{1,1}},enabled{{1,1}};uint64_t age=0;uint32_t random=1;
};
class Engine{
public:
 Parameters parameters;const Sample*sample=nullptr;bool clipped=false;double peak=0;
 void prepare(double sr){rate=sr;slew=1-std::exp(-1/(rate*0.01));for(auto&v:voices)v.prepare(sr);panic();gain=dbGain(parameters.outputDb);outputTarget=gain;currentRoute=std::clamp(parameters.route,0,5);pendingRoute=currentRoute;currentRack=pendingRack=parameters.rack;routeFade=1;routeStage=0;}
 void beginBlock(){blockOffset=0;peak=0;clipped=false;outputTarget=dbGain(parameters.outputDb);int requested=parameters.rack.enabled?currentRoute:std::clamp(parameters.route,0,5);if(requested!=pendingRoute||topologyDiff(parameters.rack,pendingRack)){pendingRoute=requested;pendingRack=parameters.rack;routeStage=1;}auto p=snapshot();for(auto&v:voices)if(v.active)v.apply(p);}
 void panic(){for(auto&v:voices)v.active=false;sustain.fill(false);keyUp.fill(false);bend.fill(0);expression.fill(1);volume.fill(1);}
 void noteOn(int note,int channel,double vel,int origin=0){channel=std::clamp(channel,1,16);Voice*t=&voices[0];for(auto&v:voices){if(!v.active){t=&v;break;}if(v.serial<t->serial)t=&v;}keyUp[size_t(t-voices.data())]=false;t->origin=origin;auto p=snapshot();t->start(note,channel,vel,++counter,p,sample,bend[size_t(channel-1)]);}
 void noteOff(int note,int channel,int origin=0){for(size_t i=0;i<voices.size();++i){auto&v=voices[i];if(v.active&&v.note==note&&v.channel==channel&&v.origin==origin){keyUp[i]=true;if(origin!=0||!sustain[size_t(channel-1)])v.release();}}}
 void pitchBend(int channel,double s){bend[size_t(channel-1)]=s;for(auto&v:voices)if(v.active&&v.channel==channel)v.pitch(s);}
 void setSustain(int channel,bool on){sustain[size_t(channel-1)]=on;if(!on)for(size_t i=0;i<voices.size();++i)if(voices[i].channel==channel&&keyUp[i])voices[i].release();}
 void control(int ch,int cc,int val){if(ch<1||ch>16)return;size_t c=size_t(ch-1);if(cc==64)setSustain(ch,val>=64);else if(cc==7)volume[c]=val/127.0;else if(cc==11)expression[c]=val/127.0;else if(cc==120){for(auto&v:voices)if(v.channel==ch)v.active=false;}else if(cc==123){for(size_t i=0;i<voices.size();++i)if(voices[i].channel==ch){keyUp[i]=true;if(!sustain[c])voices[i].release();}}else if(cc==121){setSustain(ch,false);pitchBend(ch,0);expression[c]=volume[c]=1;}}
 Stereo nextStereo(){
  ++blockOffset;if(routeStage==1){routeFade=std::max(0.0,routeFade-1/(rate*0.004));if(routeFade==0){currentRoute=pendingRoute;currentRack=pendingRack;auto p=snapshot();for(auto&v:voices)if(v.active)v.apply(p);routeStage=2;}}else if(routeStage==2){routeFade=std::min(1.0,routeFade+1/(rate*0.004));if(routeFade==1)routeStage=0;}
  Stereo out;for(auto&v:voices)if(v.active){auto s=v.nextStereo();double scale=expression[size_t(v.channel-1)]*volume[size_t(v.channel-1)];out.left+=s.left*scale;out.right+=s.right*scale;}
  gain+=slew*(outputTarget-gain);out.left*=gain*routeFade;out.right*=gain*routeFade;double pk=std::max(std::abs(out.left),std::abs(out.right));peak=std::max(peak,pk);if(pk>.98)clipped=true;out.left=std::clamp(out.left,-.98,.98);out.right=std::clamp(out.right,-.98,.98);return out;
 }
 double next(){return nextStereo().mono();}

 int activeVoices()const{int n=0;for(auto&v:voices)if(v.active)++n;return n;}
private:
 static bool topologyDiff(const RackConfig&a,const RackConfig&b){if(!a.enabled&&!b.enabled)return false;if(a.enabled!=b.enabled||a.order!=b.order)return true;for(size_t i=0;i<4;++i)if(a.modules[i].type!=b.modules[i].type)return true;return false;}
 Parameters snapshot()const{auto p=parameters;if(p.rack.clockRunning)p.rack.ppq+=double(blockOffset)*p.rack.bpm/(60*rate);p.route=currentRoute;p.rack.enabled=currentRack.enabled;p.rack.order=currentRack.order;for(size_t i=0;i<4;++i)p.rack.modules[i].type=currentRack.modules[i].type;p.preparedTail=rackTailSeconds(p.rack);return p;}
 RackConfig currentRack,pendingRack;std::array<Voice,16>voices;std::array<bool,16>sustain{},keyUp{};std::array<double,16>bend{},expression{},volume{};
 uint64_t counter=0,blockOffset=0;double rate=48000,slew=0.001,gain=1,outputTarget=1,routeFade=1;int currentRoute=0,pendingRoute=0,routeStage=0;
};
}
