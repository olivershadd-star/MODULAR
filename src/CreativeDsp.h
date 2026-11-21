#pragma once
#include <array>
#include <complex>
#include <cmath>
#include <algorithm>
namespace anks {
// Original DSP implementations. Fixed storage; prepare/reset outside processing.
class FrequencyShift {
 static constexpr int length=127,centre=63;
 std::array<double,length> ring{},kernel{};
 int write=0;double rate=48000,phase=0,shift=0,slew=.001,lastDry=0;
public:
 void prepare(double sr){rate=sr;slew=1-std::exp(-1/(rate*.01));for(int k=0;k<length;++k){int m=k-centre;double w=.42-.5*std::cos(2*pi*k/(length-1))+.08*std::cos(4*pi*k/(length-1));kernel[size_t(k)]=(m&&m%2)?2*w/(pi*m):0;}reset();}
 void reset(){ring.fill(0);write=0;phase=0;shift=0;lastDry=0;}
 double dry()const{return lastDry;}
 double next(double x,double hz){ring[size_t(write)]=x;double imaginary=0;for(int k=0;k<length;k+=2){int idx=write-k;if(idx<0)idx+=length;imaginary+=kernel[size_t(k)]*ring[size_t(idx)];}int realIndex=write-centre;if(realIndex<0)realIndex+=length;double real=ring[size_t(realIndex)];lastDry=real;if(++write==length)write=0;shift+=slew*(hz-shift);double y=real*std::cos(phase)-imaginary*std::sin(phase);phase+=2*pi*shift/rate;phase-=2*pi*std::floor(phase/(2*pi));return y;}
};
class SpectralHold {
 static constexpr int n=1024,hop=256,bins=n/2+1;
 struct Tables {
  std::array<double,n>window{};std::array<std::complex<double>,n>twiddle{};std::array<int,n>reverse{};
  Tables(){for(int i=0;i<n;++i){window[size_t(i)]=.5-.5*std::cos(2*pi*i/n);twiddle[size_t(i)]=std::polar(1.,-2*pi*i/n);int x=i,r=0;for(int b=0;b<10;++b){r=(r<<1)|(x&1);x>>=1;}reverse[size_t(i)]=r;}}
 };
 static const Tables&sharedTables(){static const Tables value;return value;}
 const Tables*tables=nullptr;
 std::array<double,n> input{},output{};
 std::array<std::complex<double>,n> frame{};
 std::array<double,n> previousWindow{};bool pendingAnalysis=false;
 std::array<double,bins> previous{},magnitude{},advance{},phase{};
 int position=0,tick=0,filled=0;bool analysed=false;double lastDry=0;bool captured=false,lastHold=false;
 static double wrap(double x){return x-2*pi*std::floor((x+pi)/(2*pi));}
 void fft(bool inverse){for(int i=0;i<n;++i)if(i<tables->reverse[size_t(i)])std::swap(frame[size_t(i)],frame[size_t(tables->reverse[size_t(i)])]);for(int size=2;size<=n;size*=2)for(int base=0;base<n;base+=size)for(int j=0;j<size/2;++j){auto w=tables->twiddle[size_t(j*n/size)];if(inverse)w=std::conj(w);auto a=frame[size_t(base+j)],b=frame[size_t(base+j+size/2)]*w;frame[size_t(base+j)]=a+b;frame[size_t(base+j+size/2)]=a-b;}if(inverse)for(auto&x:frame)x/=n;}
 void processFrame(bool hold){
  const bool take=hold&&!captured&&analysed;
  if(hold&&captured){
   // Keep the last input window, but avoid its FFT while the frozen spectrum is used.
   // A rapid off/on between hops can still recover the exact previous analysis.
   for(int i=0;i<n;++i)previousWindow[size_t(i)]=input[size_t((position+i)%n)]*tables->window[size_t(i)];
   pendingAnalysis=true;
   for(int k=0;k<bins;++k){phase[size_t(k)]=wrap(phase[size_t(k)]+advance[size_t(k)]);frame[size_t(k)]=std::polar(magnitude[size_t(k)],phase[size_t(k)]);}
  }else{
   if(take&&pendingAnalysis){for(int i=0;i<n;++i)frame[size_t(i)]={previousWindow[size_t(i)],0};fft(false);for(int k=0;k<bins;++k)previous[size_t(k)]=std::arg(frame[size_t(k)]);}
   pendingAnalysis=false;
   for(int i=0;i<n;++i)frame[size_t(i)]={input[size_t((position+i)%n)]*tables->window[size_t(i)],0};
   fft(false);
   for(int k=0;k<bins;++k){double angle=std::arg(frame[size_t(k)]),expected=2*pi*k*hop/n;double delta=expected+wrap(angle-previous[size_t(k)]-expected);previous[size_t(k)]=angle;if(take){magnitude[size_t(k)]=std::abs(frame[size_t(k)]);advance[size_t(k)]=delta;phase[size_t(k)]=angle;frame[size_t(k)]=std::polar(magnitude[size_t(k)],phase[size_t(k)]);}}
   if(take)captured=true;
  }
  analysed=true;
  frame[0]={frame[0].real(),0};frame[n/2]={frame[n/2].real(),0};for(int k=1;k<n/2;++k)frame[size_t(n-k)]=std::conj(frame[size_t(k)]);
  fft(true);for(int i=0;i<n;++i)output[size_t((position+i)%n)]+=frame[size_t(i)].real()*tables->window[size_t(i)]*(2.0/3.0);
 }

public:
 void prepare(){tables=&sharedTables();reset();}
 void reset(){input.fill(0);output.fill(0);previous.fill(0);magnitude.fill(0);advance.fill(0);phase.fill(0);position=tick=filled=0;captured=lastHold=analysed=pendingAnalysis=false;lastDry=0;}
 double dry()const{return lastDry;}
 double next(double x,bool hold){if(hold!=lastHold){if(!hold)captured=false;lastHold=hold;}double y=output[size_t(position)];output[size_t(position)]=0;lastDry=input[size_t(position)];input[size_t(position)]=x;position=(position+1)%n;filled=std::min(n,filled+1);if(++tick==hop){tick=0;if(filled==n)processFrame(hold);}return y;}
};
class SemiModular {
 double rate=48000,env=0,lfo=0,follow=0,frequency=6000,a1=0,a2=0,b1=0,b2=0,attack=1,decay=.001,release=.001,followStep=.001,cutoffStep=.001,driveGain=1;double cachedRate=-1,cachedAttack=-1,cachedDecay=-1,cachedRelease=-1,cachedDrive=-1;int stage=0;bool gate=false;
public:
 void prepare(double sr){rate=sr;followStep=1-std::exp(-1/(rate*.02));cutoffStep=1-std::exp(-1/(rate*.005));reset();}
 void configure(const RackParameters&p){bool rateChanged=rate!=cachedRate;if(rateChanged||p.smAttack!=cachedAttack)attack=1/(rate*std::max(.001,p.smAttack));if(rateChanged||p.smDecay!=cachedDecay)decay=1-std::exp(-6.907755/(rate*std::max(.005,p.smDecay)));if(rateChanged||p.smRelease!=cachedRelease)release=std::exp(-11.512925/(rate*std::max(.005,p.smRelease)));if(rateChanged||p.smDrive!=cachedDrive)driveGain=dbGain(p.smDrive);cachedRate=rate;cachedAttack=p.smAttack;cachedDecay=p.smDecay;cachedRelease=p.smRelease;cachedDrive=p.smDrive;}
 void reset(){env=lfo=follow=a1=a2=b1=b2=0;frequency=std::min(6000.,rate*.4);stage=0;gate=false;}
 void setGate(bool on){if(on&&!gate)stage=1;if(!on&&gate)stage=4;gate=on;}
 double next(double x,const RackParameters&p){
  if(stage==1){env+=attack;if(env>=1){env=1;stage=2;}}
  else if(stage==2){env+=(p.smSustain-env)*decay;if(std::abs(env-p.smSustain)<1e-5)stage=3;}
  else if(stage==3)env=p.smSustain;
  else if(stage==4){env*=release;if(env<1e-6){env=0;stage=0;}}
  lfo+=p.smLfoRate/rate;lfo-=std::floor(lfo);double wave=p.smLfoShape?1-4*std::abs(lfo-.5):std::sin(2*pi*lfo);
  follow+=(std::abs(x)-follow)*followStep;
  double cutoffMod=0,ampMod=0;
  for(int r=0;r<2;++r){double source=p.smSource[size_t(r)]==1?wave:p.smSource[size_t(r)]==2?env:p.smSource[size_t(r)]==3?std::min(1.,follow):0;double value=source*p.smAmount[size_t(r)];if(p.smDestination[size_t(r)]==0)cutoffMod+=value*5;else ampMod+=value;}
  double target=std::clamp(p.smCutoff*std::exp2(std::clamp(cutoffMod,-10.,10.)),20.,rate*.4);frequency+=(target-frequency)*cutoffStep;
  double g=std::tan(pi*frequency/rate),k=2-1.9*p.smResonance,c=1/(1+g*(g+k));
  auto filter=[&](double in,double&s1,double&s2){double v1=c*(s1+g*(in-s2)),v2=s2+g*v1;s1=2*v1-s1;s2=2*v2-s2;return p.smHighpass?in-k*v1-v2:v2;};
  double y=filter(std::tanh(x*driveGain),a1,a2);y=filter(y,b1,b2);
  double amplitude=std::clamp((p.smDrone?1.:env)+ampMod,0.,1.);return y*amplitude;
 }
};
}
