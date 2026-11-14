#pragma once
#include "RackTypes.h"
#include "CreativeDsp.h"
namespace anks {
// Storage is allocated by prepare; reset invalidates history without clearing large rings.
class History {
public:
 void prepare(size_t size){data.assign(size,0);reset();}
 void reset(){total=0;write=0;}
 void push(double x){data[write]=float(x);if(++write==data.size())write=0;++total;}
 double absolute(double pos)const{
  if(pos<0||pos>=double(total-std::min<uint64_t>(total,1))||pos<double(total>data.size()?total-data.size():0))return 0;
  auto i=uint64_t(pos);double f=pos-double(i);
  // Valid history is at most one ring behind the write position.
  auto a=int64_t(write)-int64_t(total-i);if(a<0)a+=int64_t(data.size());
  auto b=size_t(a)+1;if(b==data.size())b=0;
  return data[size_t(a)]*(1-f)+data[b]*f;
 }
 double behind(double samples)const{return absolute(double(total)-std::clamp(samples,2.0,double(data.size()-2)));}
 uint64_t count()const{return total;}
private:std::vector<float>data;uint64_t total=0;size_t write=0;
};
struct PeakEq {
 double b0=1,b1=0,b2=0,a1=0,a2=0,z1=0,z2=0;
 void reset(){z1=z2=0;}
 void configure(double sr,double frequency,double gain,double q){
  double w=2*pi*std::clamp(frequency,20.0,sr*.45)/sr,A=std::pow(10.,gain/40),alpha=std::sin(w)/(2*q),a0=1+alpha/A;
  b0=(1+alpha*A)/a0;b1=-2*std::cos(w)/a0;b2=(1-alpha*A)/a0;a1=b1;a2=(1-alpha/A)/a0;
 }
 double next(double x){double y=b0*x+z1;z1=b1*x-a1*y+z2;z2=b2*x-a2*y;return y;}
};
class RackModule {
 struct Grain {double read=0,start=0,end=1,step=1;int age=0,length=1;bool active=false;};
public:
 void setGate(bool gate){semi.setGate(gate);}
 void prepare(double sr,bool right=false){shifter.prepare(sr);freeze.prepare();semi.prepare(sr);rightChannel=right;rate=sr;slew=1-std::exp(-1/(sr*.01));for(auto&r:body)r.prepare(sr);history.prepare(size_t(sr*2.1)+8);reset();}
 void reset(){shifter.reset();if(params.type==8)freeze.reset();semi.reset();for(auto&r:body)r.reset();for(auto&e:eq)e.reset();history.reset();for(auto&g:grains)g.active=false;sv1=sv2=toneState=0;phase=0;spawn=0;firstGrain=true;random=0x9182ab31u;}
 void configure(const RackParameters&p,double hz,double bpm,double ppq,bool first=false,bool updateClock=true){
  if(p.type!=params.type){reset();if(p.type==8)freeze.reset();first=true;}
  bool tune=first;for(size_t i=0;i<2;++i){auto&a=p.body[i];auto&b=params.body[i];tune=tune||a.decay!=b.decay||a.brightness!=b.brightness||a.position!=b.position||a.detune!=b.detune||a.odd!=b.odd;}
  if(hz!=frequency)tune=true;
  const bool updateEq=first||p.type!=params.type||p.filterMode!=params.filterMode||p.eqFrequency!=params.eqFrequency||p.eqGain!=params.eqGain||p.eqQ!=params.eqQ;
  frequency=hz;params=p;if(p.type==9)semi.configure(p);
  if(tune&&p.type==1)for(size_t i=0;i<2;++i){const auto&b=p.body[i];BodyParameters bp;bp.decay=b.decay;bp.brightness=b.brightness;bp.position=b.position;bp.detune=b.detune;bp.odd=b.odd;body[i].tune(hz*std::exp2(b.detune/1200),bp);}
  if(updateEq&&p.type==2&&p.filterMode==4)for(size_t i=0;i<4;++i)eq[i].configure(rate,p.eqFrequency[i],p.eqGain[i],p.eqQ[i]);
  driveTarget=dbGain(p.drive);levelTarget=dbGain(p.level);toneCoefficient=1-std::exp(-2*pi*std::clamp(p.tone,30.,rate*.45)/rate);for(size_t i=0;i<2;++i){bodyLevelTarget[i]=dbGain(p.body[i].level);bodyEnabledTarget[i]=p.body[i].on?1:0;if(first){bodyLevel[i]=bodyLevelTarget[i];bodyEnabled[i]=bodyEnabledTarget[i];}}
  speed=p.lfoSync?bpm/(60*p.lfoBeats):p.lfoRate;if(p.lfoSync&&updateClock)phase=ppq/p.lfoBeats-std::floor(ppq/p.lfoBeats);
  target=p.on&&p.type!=0?1:0;if(first){wet=target;mix=p.mix;balance=p.balance;cutoff=p.cutoff;drive=dbGain(p.drive);level=dbGain(p.level);delay=p.time*rate;}
 }
 double next(double x){
  if(params.type==0)return x;
  wet+=slew*(target-wet);mix+=slew*(params.mix-mix);balance+=slew*(params.balance-balance);cutoff+=slew*(params.cutoff-cutoff);drive+=slew*(driveTarget-drive);level+=slew*(levelTarget-level);
  double y=x,alignedDry=x;
  switch(static_cast<ModuleType>(params.type)){
   case ModuleType::Empty:break;
   case ModuleType::Resonator:{for(size_t i=0;i<2;++i){bodyLevel[i]+=slew*(bodyLevelTarget[i]-bodyLevel[i]);bodyEnabled[i]+=slew*(bodyEnabledTarget[i]-bodyEnabled[i]);}double a=body[0].next(x)*bodyLevel[0],b=body[1].next(x)*bodyLevel[1];double wa=bodyEnabled[0]*(1-balance*bodyEnabled[1]),wb=bodyEnabled[1]*(1-(1-balance)*bodyEnabled[0]);double active=bodyEnabled[0]+bodyEnabled[1]-bodyEnabled[0]*bodyEnabled[1];y=wa*a+wb*b+(1-active)*x;break;}
   case ModuleType::Filter:{
    double lfo=params.lfoShape==1?1-4*std::abs(phase-.5):std::sin(2*pi*phase);phase+=speed/rate;phase-=std::floor(phase);
    double fc=std::clamp(cutoff*std::exp2(params.lfoDepth*lfo),20.,rate*.45),g=std::tan(pi*fc/rate),k=1/std::max(.5,params.q),a1=1/(1+g*(g+k));
    double v1=a1*(sv1+g*(x-sv2)),v2=sv2+g*v1;sv1=2*v1-sv1;sv2=2*v2-sv2;
    if(params.filterMode==0)y=v2;else if(params.filterMode==1)y=x-k*v1-v2;else if(params.filterMode==2)y=v1;else if(params.filterMode==3)y=x-k*v1;else{y=x;for(auto&e:eq)y=e.next(y);}break;}
   case ModuleType::Saturation:{double driven=x*drive;double z=params.distoMode==1?std::clamp(driven,-1.,1.):params.distoMode==2?(2/pi)*std::asin(std::sin(driven*pi*.5)):std::tanh(driven);toneState+=toneCoefficient*(z-toneState);y=toneState;break;}
   case ModuleType::Delay:{delay+=slew*(params.time*rate-delay);y=history.behind(delay);toneState+=toneCoefficient*(y-toneState);history.push(x+std::tanh(toneState)*params.feedback);break;}
   case ModuleType::Chorus:{
    phase+=params.rate/rate;phase-=std::floor(phase);history.push(x);y=0;
    for(int tap=0;tap<3;++tap){double angle=2*pi*phase+tap*2*pi/3+(rightChannel?pi*.5:0);double seconds=.014+tap*.004+(.001+.010*params.depth)*std::sin(angle);y+=history.behind(rate*seconds)/3;}
    break;}
   case ModuleType::FrequencyShifter:y=shifter.next(x,params.shiftHz);alignedDry=shifter.dry();break;
   case ModuleType::SpectralFreeze:y=freeze.next(x,params.freezeHold);alignedDry=freeze.dry();break;
   case ModuleType::SemiModular:y=semi.next(x,params);break;
   case ModuleType::Granular:{
    history.push(x);
    // Capture real recorded fragments. Never start a grain in negative/unwritten history.
    // A 5 ms capture gives an impact an audible wet onset without a dry bypass.
    const double recorded=double(history.count());
    if(recorded>=rate*.005&&(--spawn<=0||firstGrain)){
     spawn=rate/std::max(1.,params.density);firstGrain=false;
     for(auto&g:grains)if(!g.active){
      g.active=true;g.age=0;g.length=std::max(2,int(params.grainSize*rate));g.step=std::exp2(params.pitch/12);
      double oldest=std::max(0.,recorded-rate*2.0),latest=recorded-1;
      double jitter=(rand01()*2-1)*params.scatter*.25*rate;
      double desired=latest-params.position*rate+jitter;
      // At startup the requested lookback is clamped to the material actually available.
      g.start=std::clamp(desired-g.length*g.step,oldest,std::max(oldest,latest-2));
      g.end=std::min(latest,g.start+g.length*g.step);g.read=g.start;break;
     }
    }
    y=0;double weight=0;
    for(auto&g:grains)if(g.active){
     double attack=std::min(rate*.001,g.length*.1),release=std::max(1.,g.length*.25);
     double w=std::min(1.,double(g.age+1)/std::max(1.,attack))*std::min(1.,double(g.length-g.age)/release);
     y+=history.absolute(g.read)*w;weight+=w;
     g.read+=g.step;double span=std::max(1.,g.end-g.start);if(g.read>=g.end)g.read=g.start+std::fmod(g.read-g.start,span);
     if(++g.age>=g.length)g.active=false;
    }
    y/=std::max(1.,weight);break;}

  }
  if(params.type==7||params.type==8)return std::clamp(x+wet*(level*(alignedDry+mix*(y-alignedDry))-x),-16.,16.);
  // Bound interstage extremes before the next module, never a feedback network.
  return std::clamp(x+wet*(mix*(y-x)+(level-1)*(x+mix*(y-x))),-16.,16.);
 }
private:
 double rand01(){random^=random<<13;random^=random>>17;random^=random<<5;return random/4294967295.;}
 double driveTarget=2,levelTarget=1,toneCoefficient=.5;std::array<double,2>bodyLevel{{1,1}},bodyLevelTarget{{1,1}},bodyEnabled{{1,1}},bodyEnabledTarget{{1,1}};
 bool rightChannel=false,firstGrain=true;
 FrequencyShift shifter;SpectralHold freeze;SemiModular semi;
 RackParameters params;std::array<Resonator,2>body;std::array<PeakEq,4>eq;History history;std::array<Grain,16>grains{};
 double rate=48000,slew=.001,frequency=0,wet=1,target=1,mix=.8,balance=.5,cutoff=6000,drive=2,level=1,delay=12000,sv1=0,sv2=0,toneState=0,phase=0,speed=.5,spawn=0;uint32_t random=0x9182ab31u;
};
}

namespace anks {
// Independent channel states preserve stereo from a chorus through the later blocks.
class StereoRackModule {
public:
 void setGate(bool gate){left.setGate(gate);right.setGate(gate);}
 void prepare(double sr){left.prepare(sr);right.prepare(sr,true);}
 void reset(){left.reset();right.reset();}
 void configure(const RackParameters&p,double hz,double bpm,double ppq,bool first=false,bool clock=true){left.configure(p,hz,bpm,ppq,first,clock);right.configure(p,hz,bpm,ppq,first,clock);}
 Stereo next(Stereo x){return {left.next(x.left),right.next(x.right)};}
private: RackModule left,right;
};
inline double rackTailSeconds(const RackConfig&r){
 double seconds=0;for(auto&p:r.modules)if(p.on){
  if(p.type==5){double repeats=p.feedback<.001?1:1+std::log(1e-5)/std::log(std::min(.95,p.feedback));seconds+=std::min(30.,p.time*repeats);}
  else if(p.type==4)seconds+=p.position+p.grainSize+.05;
  else if(p.type==6)seconds+=.08;
  else if(p.type==7)seconds+=.01;
  else if(p.type==8)seconds+=p.freezeHold?30.:.05;
  else if(p.type==9)seconds+=p.smRelease;
  else if(p.type==1)for(auto&b:p.body)if(b.on)seconds=std::max(seconds,b.decay);
 }return std::min(30.,seconds);
}
}
