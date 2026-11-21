#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "Engine.h"
#include "Sequencer.h"
#include "RackSchema.h"
#include "FactorySources.h"
class ResonantLab final:public juce::AudioProcessor {
public:
 ResonantLab();
 const juce::String getName()const override{return "ANKS Resonant Lab";}
 void prepareToPlay(double,int)override;void releaseResources()override{engine.panic();sequence.reset();}
 bool isBusesLayoutSupported(const BusesLayout&l)const override{return l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo();}
 bool acceptsMidi()const override{return true;}bool producesMidi()const override{return false;}
 double getTailLengthSeconds()const override{return 35;}
 bool hasEditor()const override{return true;}juce::AudioProcessorEditor*createEditor()override;
 int getNumPrograms()override{return 1;}int getCurrentProgram()override{return 0;}
 void setCurrentProgram(int)override{}const juce::String getProgramName(int)override{return "Initial";}
 void changeProgramName(int,const juce::String&)override{}
 void getStateInformation(juce::MemoryBlock&)override;void setStateInformation(const void*,int)override;
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&)override;
 bool saveRecipe(const juce::File&,bool module,int identity=0);bool loadRecipe(const juce::File&,bool module,int identity=0);
 void swapRackSlots(int,int);void convertLegacy();
 bool savePreset(const juce::File&);bool loadPreset(const juce::File&);
 juce::String loadFactory(int);juce::String loadSampleBytes(juce::MemoryBlock,const juce::String&);
 juce::String loadSample(const juce::File&);juce::String sampleLabel();
 juce::AudioProcessorValueTreeState state;
 juce::MidiKeyboardState keyboard;
 std::atomic<float>currentBpm{120};
 std::atomic<int> sequenceStep{-1},clockStatus{0};
 std::atomic<float>audioLoad{0},audioLoadPeak{0};std::atomic<uint64_t>lateCallbacks{0},blockedCallbacks{0};
 std::atomic<float> meterPeak{0};std::atomic<bool>clipLatch{false};
 void setRouteOrder(const std::array<int,3>&);
 void editBodyParameter(size_t,float);
private:
 bool restoreState(const void*,int);void readRack();
 std::array<std::atomic<float>*,11>fmPointers{};
 std::atomic<float>*rackEnabled=nullptr,*rackOrder=nullptr,*wavePointer=nullptr;
 std::array<std::atomic<float>*,4>rackTypes{};std::array<std::array<std::atomic<float>*,anks::rackFields.size()>,4>rackPointers{};
 double freePpq=0,audioRate=48000;
 static juce::AudioProcessorValueTreeState::ParameterLayout layout();
 float value(const char*id)const{return state.getRawParameterValue(id)->load();}
 anks::SequenceConfig readSequence()const;
 void updateParameters();void handle(const juce::MidiMessage&);void render(juce::AudioBuffer<float>&,int,int);
 juce::String decode(const juce::MemoryBlock&,std::unique_ptr<anks::Sample>&);
 std::array<std::array<std::atomic<float>*,7>,2>bodyPointers{};std::array<std::atomic<float>*,15>sharedPointers{};
 std::array<std::array<std::atomic<float>*,4>,16>stepPointers{};std::array<std::atomic<float>*,7>sequencePointers{};
 anks::StepSequencer sequence;
 anks::Engine engine;std::unique_ptr<anks::Sample> source;
 juce::CriticalSection sampleStateLock;bool droppedBlock=false;
 juce::MemoryBlock sourceBytes;juce::String sourceName="Impact synthetique interne";
};
