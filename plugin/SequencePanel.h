#pragma once
#include "PluginProcessor.h"
class SequencePanel final:public juce::Component,private juce::Timer{
 using State=juce::AudioProcessorValueTreeState;
public:
 explicit SequencePanel(ModularProcessor&p):processor(p){
  addAndMakeVisible(enabled);enabled.setButtonText("SEQUENCEUR ACTIF");enabledAttachment=std::make_unique<State::ButtonAttachment>(p.state,"seq_on",enabled);
  addAndMakeVisible(clock);clock.addItem("Hote",1);clock.addItem("Interne",2);clockAttachment=std::make_unique<State::ComboBoxAttachment>(p.state,"seq_clock",clock);
  addAndMakeVisible(run);run.setClickingTogglesState(true);runAttachment=std::make_unique<State::ButtonAttachment>(p.state,"seq_run",run);
  setup(tempo);tempoAttachment=std::make_unique<State::SliderAttachment>(p.state,"seq_bpm",tempo);
  setup(length);lengthAttachment=std::make_unique<State::SliderAttachment>(p.state,"seq_length",length);
  addAndMakeVisible(division);division.addItemList(juce::StringArray{"1/4","1/8","1/16","1/32"},1);divisionAttachment=std::make_unique<State::ComboBoxAttachment>(p.state,"seq_division",division);
  setup(channel);channelAttachment=std::make_unique<State::SliderAttachment>(p.state,"seq_channel",channel);
  addAndMakeVisible(status);status.setColour(juce::Label::textColourId,juce::Colour(0xff91a2be));
  const char*names[]={"HORLOGE","INTERNE","BPM INTERNE","PAS","RESOLUTION","CANAL"};
  for(size_t i=0;i<labels.size();++i){addAndMakeVisible(labels[i]);labels[i].setText(names[i],juce::dontSendNotification);labels[i].setColour(juce::Label::textColourId,juce::Colour(0xffedf2ff));labels[i].setFont(juce::Font{juce::FontOptions{12.f}});}
  for(size_t i=0;i<16;++i){auto prefix="seq_"+juce::String(int(i+1)).paddedLeft('0',2)+"_";addAndMakeVisible(stepOn[i]);stepOn[i].setButtonText(juce::String(int(i+1)).paddedLeft('0',2));stepOnAttachment[i]=std::make_unique<State::ButtonAttachment>(p.state,prefix+"on",stepOn[i]);
   for(size_t k=0;k<3;++k){setup(values[i][k]);values[i][k].setTextBoxStyle(juce::Slider::TextBoxLeft,false,42,23);const char*ids[]={"note","velocity","gate"};valueAttachments[i][k]=std::make_unique<State::SliderAttachment>(p.state,prefix+ids[k],values[i][k]);}}
  timerCallback();startTimerHz(20);
 }
 int preferredHeight()const{return getWidth()<900?709:445;}
 void paint(juce::Graphics&g)override{
  g.setColour(juce::Colour(0xff1c2738));g.fillRoundedRectangle(getLocalBounds().toFloat(),10);g.setColour(juce::Colour(0xff7ce2ed));g.setFont(19);g.drawText("SEQUENCEUR / 16 PAS",14,8,310,28,juce::Justification::centredLeft);
  g.setFont(12);g.setColour(juce::Colour(0xff91a2be));g.drawText("N : note MIDI    V : velocite    % : duree dans le pas",16,125,getWidth()-32,22,juce::Justification::centredLeft);
  for(int i=0;i<16;++i){auto r=cell(i);g.setColour(juce::Colour(0xff101723));g.fillRoundedRectangle(r.toFloat(),6);if(i==current){g.setColour(juce::Colour(0xffffae70));g.drawRoundedRectangle(r.toFloat().reduced(1),6,2);}g.setColour(juce::Colour(0xff91a2be));g.setFont(11);g.drawText("N",r.getX()+3,r.getY()+29,16,22,juce::Justification::centred);g.drawText("V",r.getX()+3,r.getY()+58,16,22,juce::Justification::centred);g.drawText("%",r.getX()+3,r.getY()+87,16,22,juce::Justification::centred);}
 }
 void resized()override{
  enabled.setBounds(getWidth()-224,9,210,28);
  const int xs[]={16,136,240,410,505,615},widths[]={110,92,155,85,95,85};
  for(size_t i=0;i<6;++i)labels[i].setBounds(xs[i],46,widths[i],18);
  clock.setBounds(xs[0],68,widths[0],28);run.setBounds(xs[1],68,widths[1],28);tempo.setBounds(xs[2],68,widths[2],28);length.setBounds(xs[3],68,widths[3],28);division.setBounds(xs[4],68,widths[4],28);channel.setBounds(xs[5],68,widths[5],28);status.setBounds(14,100,getWidth()-28,24);
  for(int i=0;i<16;++i){auto r=cell(i);stepOn[size_t(i)].setBounds(r.getX()+2,r.getY()+1,r.getWidth()-4,24);for(int k=0;k<3;++k)values[size_t(i)][size_t(k)].setBounds(r.getX()+21,r.getY()+28+k*29,r.getWidth()-24,25);}
 }
private:
 ModularProcessor&processor;juce::ToggleButton enabled;juce::TextButton run;juce::ComboBox clock,division;juce::Slider tempo,length,channel;juce::Label status;std::array<juce::Label,6>labels;
 std::unique_ptr<State::ButtonAttachment>enabledAttachment,runAttachment;std::unique_ptr<State::ComboBoxAttachment>clockAttachment,divisionAttachment;std::unique_ptr<State::SliderAttachment>tempoAttachment,lengthAttachment,channelAttachment;
 std::array<juce::ToggleButton,16>stepOn;std::array<std::unique_ptr<State::ButtonAttachment>,16>stepOnAttachment;
 std::array<std::array<juce::Slider,3>,16>values;std::array<std::array<std::unique_ptr<State::SliderAttachment>,3>,16>valueAttachments;
 int current=-1;
 juce::Rectangle<int>cell(int i)const{int columns=getWidth()<900?4:8;int w=(getWidth()-30)/columns;return {15+(i%columns)*w,155+(i/columns)*132,w-5,121};}
 void setup(juce::Slider&s){addAndMakeVisible(s);s.setSliderStyle(juce::Slider::IncDecButtons);s.setTextBoxStyle(juce::Slider::TextBoxLeft,false,54,23);s.setColour(juce::Slider::textBoxTextColourId,juce::Colours::white);s.setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);}
 void timerCallback()override{
  bool internal=processor.state.getRawParameterValue("seq_clock")->load()>0.5f;tempo.setEnabled(internal);run.setEnabled(internal);run.setButtonText(run.getToggleState()?"STOP":"PLAY");
  const char*texts[]={"Sequenceur coupe : jeu au clavier disponible.","Horloge interne : appuyer sur PLAY.","Horloge interne en lecture.","Transport hote absent : choisir Interne pour jouer sans hote.","Synchronise : lancer la lecture dans l'hote.","Lecture synchronisee sur le transport de l'hote."};int s=enabled.getToggleState()?juce::jlimit(0,5,processor.clockStatus.load()):0;status.setText(juce::String(texts[s])+((s==2||s==4||s==5)?" "+juce::String(processor.currentBpm.load(),1)+" BPM":""),juce::dontSendNotification);
  int next=enabled.getToggleState()?processor.sequenceStep.load():-1;if(next!=current){current=next;repaint();}
  int count=int(processor.state.getRawParameterValue("seq_length")->load());for(int i=0;i<16;++i){bool available=i<count;stepOn[size_t(i)].setEnabled(available);for(auto&slider:values[size_t(i)])slider.setEnabled(available);}
 }
};
