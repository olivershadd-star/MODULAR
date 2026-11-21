#pragma once
#include <array>
#include <algorithm>
#include <cmath>
namespace anks {
struct FmParameters {
 bool enabled=false;
 double ratio=2,index=2,feedback=0,modAttack=.005,modDecay=.6,modSustain=0,modRelease=.3,velocity=.5,ampDecay=.5,ampSustain=1;
};
// Two sine operators, phase modulation, one-sample modulator feedback.
// Fixed 2x synthesis followed by a 63-tap Blackman lowpass decimator.
class FmSource {
 static constexpr double tau=6.28318530717958647692;
 struct Envelope {
  double value=0,attack=1,decay=.001,sustain=0,sustainTarget=0,sustainStep=.001,release=.001;int stage=1;
  double cachedRate=-1,cachedAttack=-1,cachedDecay=-1,cachedRelease=-1;
  void configure(double sr,double a,double d,double s,double r,bool first){bool rateChanged=sr!=cachedRate;if(rateChanged)sustainStep=1-std::exp(-1/(sr*.005));if(rateChanged||a!=cachedAttack)attack=a<=0?1:1/(sr*a);if(rateChanged||d!=cachedDecay)decay=1-std::exp(-6.907755/(sr*std::max(.005,d)));sustainTarget=std::clamp(s,0.,1.);if(first)sustain=sustainTarget;if(rateChanged||r!=cachedRelease)release=std::exp(-11.512925/(sr*std::max(.005,r)));cachedRate=sr;cachedAttack=a;cachedDecay=d;cachedRelease=r;}

  void reset(){value=0;stage=1;}
  double next(bool held){sustain+=sustainStep*(sustainTarget-sustain);if(!held)stage=4;if(stage==1){value=std::min(1.,value+attack);if(value>=1)stage=2;}else if(stage==2){value+=(sustain-value)*decay;if(std::abs(value-sustain)<1e-6)stage=3;}else if(stage==3)value=sustain;else value*=release;return value;}
 } modEnvelope,ampEnvelope;
 std::array<double,63> history{},kernel{};
 double rate=96000,slew=.001,carrier=0,modulator=0,lastMod=0,hz=440,ratio=2,index=2,feedback=0,ratioTarget=2,indexTarget=2,feedbackTarget=0;
 int write=0;
 static double wrap(double phase){return phase-tau*std::floor(phase/tau);}
public:
 void prepare(double sr){rate=sr*2;slew=1-std::exp(-1/(rate*.01));double sum=0;for(int k=0;k<63;++k){int m=k-31;double sinc=m?std::sin(tau*.225*m)/(3.14159265358979323846*m):.45;double window=.42-.5*std::cos(tau*k/62)+.08*std::cos(2*tau*k/62);kernel[size_t(k)]=sinc*window;sum+=kernel[size_t(k)];}for(auto&v:kernel)v/=sum;reset();}
 void reset(){carrier=modulator=lastMod=0;write=0;history.fill(0);modEnvelope.reset();ampEnvelope.reset();}
 void configure(double frequency,const FmParameters&p,double attack,double release,double velocity,bool first=false){hz=std::clamp(frequency,0.,rate*.45);ratioTarget=std::clamp(p.ratio,.125,16.);indexTarget=std::clamp(p.index,0.,12.)*(1-std::clamp(p.velocity,0.,1.)*(1-std::clamp(velocity,0.,1.)));feedbackTarget=std::clamp(p.feedback,0.,.95);modEnvelope.configure(rate,p.modAttack,p.modDecay,p.modSustain,p.modRelease,first);ampEnvelope.configure(rate,attack,p.ampDecay,p.ampSustain,release,first);if(first){ratio=ratioTarget;index=indexTarget;feedback=feedbackTarget;}}
 double next(bool held){
  for(int sub=0;sub<2;++sub){ratio+=slew*(ratioTarget-ratio);index+=slew*(indexTarget-index);feedback+=slew*(feedbackTarget-feedback);double mod=std::sin(modulator+feedback*3.14159265358979323846*lastMod);lastMod=mod;double y=.25*std::sin(carrier+index*modEnvelope.next(held)*mod)*ampEnvelope.next(held);history[size_t(write)]=y;if(++write==63)write=0;carrier=wrap(carrier+tau*hz/rate);modulator=wrap(modulator+tau*std::min(hz*ratio,rate*.45)/rate);}
  double out=0;for(int k=0;k<63;++k){int at=write-1-k;if(at<0)at+=63;out+=kernel[size_t(k)]*history[size_t(at)];}return out;
 }
};
}
