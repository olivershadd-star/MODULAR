#pragma once
#include "PluginProcessor.h"
#include "SequencePanel.h"
class LabPanel final:public juce::Component,private juce::Timer{
 using State=juce::AudioProcessorValueTreeState;
public:
 explicit LabPanel(ResonantLab&p):processor(p),sequencerPanel(p){
  setSize(940,1240);addAndMakeVisible(sequencerPanel);addAndMakeVisible(load);load.setButtonText("IMPORTER WAV / AIFF");load.onClick=[this]{choose();};addAndMakeVisible(source);addAndMakeVisible(presetOpen);addAndMakeVisible(presetSave);presetOpen.setButtonText("Charger preset");presetSave.setButtonText("Sauver preset");presetOpen.onClick=[this]{choosePreset(false);};presetSave.onClick=[this]{choosePreset(true);};source.setColour(juce::Label::textColourId,juce::Colour(0xffedf2ff));
  const char*toggleIds[]={"filter_on","drive_on","bodies_on","sample","loop","a_on","b_on"};
  const char*toggleNames[]={"Filtre actif","Saturation active","Corps actifs","Lire le WAV importe","Boucler tout le WAV","Corps A actif","Corps B actif"};
  for(size_t i=0;i<toggles.size();++i){addAndMakeVisible(toggles[i]);toggles[i].setButtonText(toggleNames[i]);toggleAttach[i]=std::make_unique<State::ButtonAttachment>(p.state,toggleIds[i],toggles[i]);}
  const char*moduleIds[]={"cutoff","drive","mix","balance"};const char*moduleNames[]={"Coupure / Hz","Drive / dB","Source / corps","Balance A / B"};
  for(size_t i=0;i<moduleSliders.size();++i){setupSlider(moduleSliders[i],false);moduleAttach[i]=std::make_unique<State::SliderAttachment>(p.state,moduleIds[i],moduleSliders[i]);setupLabel(moduleLabels[i],moduleNames[i]);if(i>=2){moduleSliders[i].textFromValueFunction=[](double v){return juce::String(v*100,1)+" %";};moduleSliders[i].valueFromTextFunction=[](const juce::String&t){return t.getDoubleValue()/100;};}}
  const char*globalIds[]={"input","output","attack","release","root"};const char*globalNames[]={"ENTREE / dB","SORTIE / dB","ATTAQUE / s","RELACHEMENT / s","RACINE / MIDI"};
  for(size_t i=0;i<globalSliders.size();++i){setupSlider(globalSliders[i],true);globalAttach[i]=std::make_unique<State::SliderAttachment>(p.state,globalIds[i],globalSliders[i]);setupLabel(globalLabels[i],globalNames[i]);}
  const char*bodyNames[]={"RESONANCE / s","BRILLANCE","POSITION","ACCORD / cents","NIVEAU / dB"};
  const double mins[]={0.05,0.05,0.02,-1200,-24},maxs[]={8,1,0.48,1200,12},steps[]={0.01,0.001,0.001,0.1,0.1};
  for(size_t i=0;i<bodySliders.size();++i){auto&s=bodySliders[i];setupSlider(s,true);s.setRange(mins[i],maxs[i],steps[i]);if(i==0)s.setSkewFactor(0.4);if(i==1||i==2){s.textFromValueFunction=[](double v){return juce::String(v*100,1)+" %";};s.valueFromTextFunction=[](const juce::String&t){return t.getDoubleValue()/100;};}setupLabel(bodyLabels[i],bodyNames[i]);setupLabel(bodyValues[i],"");s.onValueChange=[this,i]{writeBody(i,float(bodySliders[i].getValue()));};}
  addAndMakeVisible(edit);edit.addItem("A",1);edit.addItem("B",2);edit.addItem("A+B LIES",3);editAttach=std::make_unique<State::ComboBoxAttachment>(p.state,"edit",edit);
  addAndMakeVisible(odd);odd.setButtonText("Resonance impaire");odd.onClick=[this]{writeBody(5,odd.getToggleState()?1.f:0.f);};
  for(size_t i=0;i<left.size();++i){addAndMakeVisible(left[i]);addAndMakeVisible(right[i]);left[i].setButtonText("<");right[i].setButtonText(">");left[i].onClick=[this,i]{moveModule(int(i),-1);};right[i].onClick=[this,i]{moveModule(int(i),1);};}
  timerCallback();startTimerHz(15);
 }
 void paint(juce::Graphics&g)override{
  g.fillAll(juce::Colour(0xff101723));auto card=[&](juce::Rectangle<int>r){g.setColour(juce::Colour(0xff1c2738));g.fillRoundedRectangle(r.toFloat(),10);};
  auto order=currentOrder();int w=(getWidth()-40)/3;
  const char*names[]={"FILTRE","SATURATION","DOUBLE CORPS"};
  for(int i=0;i<3;++i){card({12+i*(w+8),98,w,156});g.setColour(juce::Colour(0xff7ce2ed));g.setFont(17);g.drawText(juce::String(i+1)+" / "+names[order[size_t(i)]],24+i*(w+8),107,w-24,25,juce::Justification::centredLeft);}
  card({12,270,getWidth()-24,248});g.setColour(juce::Colour(0xffffae70));g.setFont(18);g.drawText("CORPS RESONANTS / EDITION",26,280,330,26,juce::Justification::centredLeft);
  g.setColour(juce::Colour(0xff91a2be));g.setFont(13);g.drawText("A / B choisit le corps edite. LIES applique le prochain geste aux deux.",26,486,getWidth()-52,22,juce::Justification::centredLeft);
  card({12,535,getWidth()-24,190});g.setColour(juce::Colour(0xff91a2be));g.setFont(13);g.drawText("Les fleches deplacent les vrais blocs. Les deux corps travaillent en parallele.",24,738,getWidth()-48,22,juce::Justification::centredLeft);
 }
 void resized()override{
  load.setBounds(16,10,270,34);source.setBounds(300,10,getWidth()-316,34);toggles[3].setBounds(16,52,240,30);toggles[4].setBounds(245,52,210,30);presetOpen.setBounds(getWidth()-270,52,122,30);presetSave.setBounds(getWidth()-136,52,122,30);int sh=(getWidth()-24)<900?709:445;sequencerPanel.setBounds(12,780,getWidth()-24,sh);if(getHeight()!=795+sh)setSize(getWidth(),795+sh);
  auto order=currentOrder();int w=(getWidth()-40)/3;
  for(int pos=0;pos<3;++pos){int module=order[size_t(pos)],x=12+pos*(w+8);toggles[size_t(module)].setBounds(x+12,137,w-90,25);left[size_t(module)].setBounds(x+w-72,136,28,25);right[size_t(module)].setBounds(x+w-38,136,28,25);left[size_t(module)].setEnabled(pos>0);right[size_t(module)].setEnabled(pos<2);
   if(module<2){moduleLabels[size_t(module)].setBounds(x+12,173,w-24,22);moduleSliders[size_t(module)].setBounds(x+10,198,w-20,36);}else{moduleLabels[2].setBounds(x+12,166,w-24,18);moduleSliders[2].setBounds(x+10,183,w-20,26);moduleLabels[3].setBounds(x+12,209,w-24,18);moduleSliders[3].setBounds(x+10,225,w-20,26);}}
  edit.setBounds(getWidth()-210,280,180,28);toggles[5].setBounds(25,315,170,26);toggles[6].setBounds(200,315,170,26);odd.setBounds(getWidth()-240,315,220,26);
  int cell=(getWidth()-40)/5;for(size_t i=0;i<5;++i){int x=20+int(i)*cell;bodyLabels[i].setBounds(x,351,cell,23);bodySliders[i].setBounds(x+5,375,cell-10,86);bodyValues[i].setBounds(x,461,cell,22);globalLabels[i].setBounds(x,548,cell,25);globalSliders[i].setBounds(x+5,580,cell-10,126);}
 }
 juce::String meterText;bool overload=false;
private:
 ResonantLab&processor;SequencePanel sequencerPanel;juce::TextButton load,presetOpen,presetSave;juce::Label source;juce::ComboBox edit;juce::ToggleButton odd;
 std::array<juce::ToggleButton,7>toggles;std::array<std::unique_ptr<State::ButtonAttachment>,7>toggleAttach;
 std::array<juce::Slider,4>moduleSliders;std::array<juce::Label,4>moduleLabels;std::array<std::unique_ptr<State::SliderAttachment>,4>moduleAttach;
 std::array<juce::Slider,5>globalSliders,bodySliders;std::array<juce::Label,5>globalLabels,bodyLabels,bodyValues;std::array<std::unique_ptr<State::SliderAttachment>,5>globalAttach;
 std::unique_ptr<State::ComboBoxAttachment>editAttach;std::array<juce::TextButton,3>left,right;std::unique_ptr<juce::FileChooser>chooser;
 int routeSeen=-1,clipHold=0;float meter=0;
 void setupSlider(juce::Slider&s,bool rotary){addAndMakeVisible(s);s.setSliderStyle(rotary?juce::Slider::RotaryHorizontalVerticalDrag:juce::Slider::LinearHorizontal);s.setTextBoxStyle(rotary?juce::Slider::TextBoxBelow:juce::Slider::TextBoxRight,false,82,22);s.setColour(juce::Slider::rotarySliderFillColourId,juce::Colour(0xff7ce2ed));s.setColour(juce::Slider::trackColourId,juce::Colour(0xff7ce2ed));s.setColour(juce::Slider::textBoxTextColourId,juce::Colours::white);s.setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);}
 void setupLabel(juce::Label&l,const char*text){addAndMakeVisible(l);l.setText(text,juce::dontSendNotification);l.setJustificationType(juce::Justification::centred);l.setColour(juce::Label::textColourId,juce::Colour(0xffedf2ff));}
 std::array<int,3>currentOrder()const{return anks::routes[size_t(juce::jlimit(0,5,int(processor.state.getRawParameterValue("route")->load())))];}
 void moveModule(int module,int direction){auto order=currentOrder();for(int i=0;i<3;++i)if(order[size_t(i)]==module){int j=i+direction;if(j>=0&&j<3){std::swap(order[size_t(i)],order[size_t(j)]);processor.setRouteOrder(order);}return;}}
 void writeBody(size_t index,float value){processor.editBodyParameter(index,value);}
 void timerCallback()override{
  source.setText(processor.sampleLabel(),juce::dontSendNotification);int route=int(processor.state.getRawParameterValue("route")->load());if(routeSeen!=route){routeSeen=route;resized();repaint();}
  int mode=int(processor.state.getRawParameterValue("edit")->load());const char*ids[]={"decay","brightness","position","detune","level"};
  for(size_t i=0;i<5;++i){float a=processor.state.getRawParameterValue(juce::String("a_")+ids[i])->load(),b=processor.state.getRawParameterValue(juce::String("b_")+ids[i])->load();if(!bodySliders[i].isMouseButtonDown())bodySliders[i].setValue(mode==1?b:a,juce::dontSendNotification);const bool percent=i==1||i==2;bodyValues[i].setText("A "+juce::String(percent?a*100:a,percent?1:2)+" / B "+juce::String(percent?b*100:b,percent?1:2)+(percent?" %":""),juce::dontSendNotification);bodySliders[i].setColour(juce::Slider::rotarySliderFillColourId,juce::Colour(mode==1?0xffffae70:0xff7ce2ed));}
  bool oddValue=processor.state.getRawParameterValue(mode==1?"b_odd":"a_odd")->load()>0.5f;odd.setToggleState(oddValue,juce::dontSendNotification);
  if(isShowing())meter=std::max(processor.meterPeak.load(),meter*0.82f);if(isShowing()&&processor.clipLatch.load())clipHold=20;else clipHold=std::max(0,clipHold-1);overload=clipHold>0;meterText=meter>0.00001f?juce::String(20*std::log10(meter),1)+" dBFS":"silence";
  if(auto*parent=findParentComponentOfClass<juce::AudioProcessorEditor>())parent->repaint(0,0,parent->getWidth(),55);
 }
 void choosePreset(bool save){
  chooser=std::make_unique<juce::FileChooser>(save?"Sauver le preset complet":"Charger un preset complet",juce::File{},"*.anksres");juce::Component::SafePointer<LabPanel>safe(this);
  int flags=(save?juce::FileBrowserComponent::saveMode:juce::FileBrowserComponent::openMode)|juce::FileBrowserComponent::canSelectFiles;if(save)flags|=juce::FileBrowserComponent::warnAboutOverwriting;
  chooser->launchAsync(flags,[safe,save](const juce::FileChooser&c){if(safe==nullptr||c.getResult()==juce::File{})return;auto file=c.getResult();if(save&&!file.hasFileExtension("anksres")){juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,"Preset","Ajouter l'extension .anksres au nom du fichier.");return;}bool ok=save?safe->processor.savePreset(file):safe->processor.loadPreset(file);if(!ok)juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,"Preset","Preset illisible ou ecriture impossible.");});
 }
 void choose(){chooser=std::make_unique<juce::FileChooser>("Importer une excitation (10 s maximum)",juce::File{},"*.wav;*.aif;*.aiff");juce::Component::SafePointer<LabPanel>safe(this);chooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,[safe](const juce::FileChooser&c){if(safe==nullptr||c.getResult()==juce::File{})return;auto error=safe->processor.loadSample(c.getResult());if(error.isNotEmpty())juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,"Import",error);});}
};
class LabEditor final:public juce::AudioProcessorEditor{
public:
 explicit LabEditor(ResonantLab&p):AudioProcessorEditor(p),panel(p),keys(p.keyboard,juce::MidiKeyboardComponent::horizontalKeyboard){setResizable(true,true);setResizeLimits(760,520,1600,1100);setSize(960,700);addAndMakeVisible(view);view.setViewedComponent(&panel,false);view.setScrollBarsShown(true,false);addAndMakeVisible(keys);addAndMakeVisible(soundPage);addAndMakeVisible(sequencePage);soundPage.setButtonText("SON");sequencePage.setButtonText("SEQUENCE");soundPage.onClick=[this]{view.setViewPosition(0,0);};sequencePage.onClick=[this]{view.setViewPosition(0,780);};keys.setAvailableRange(24,96);keys.setLowestVisibleKey(36);}
 void paint(juce::Graphics&g)override{g.fillAll(juce::Colour(0xff101723));g.setColour(juce::Colour(0xff7ce2ed));g.setFont(22);g.drawText("ANKS / RESONANT LAB V1",20,10,getWidth()-390,34,juce::Justification::centredLeft);g.setColour(panel.overload?juce::Colour(0xffff8796):juce::Colour(0xffedf2ff));g.setFont(14);g.drawText((panel.overload?"CRETE !  ":"SORTIE  ")+panel.meterText,getWidth()-170,12,150,30,juce::Justification::centredRight);}
 void resized()override{view.setBounds(0,55,getWidth(),getHeight()-143);panel.setSize(getWidth()-view.getScrollBarThickness(),1240);soundPage.setBounds(getWidth()-350,12,60,30);sequencePage.setBounds(getWidth()-280,12,100,30);keys.setBounds(16,getHeight()-76,getWidth()-32,62);keys.setKeyWidth(float(getWidth()-32)/43.f);}
private:
 LabPanel panel;juce::Viewport view;juce::TextButton soundPage,sequencePage;juce::MidiKeyboardComponent keys;
};
