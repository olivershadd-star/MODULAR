#include "PluginProcessor.h"
#include "RackEditor.h"
#include "SampleIO.h"
#include <limits>
ResonantLab::ResonantLab():AudioProcessor(BusesProperties().withOutput("Output",juce::AudioChannelSet::stereo(),true)),state(*this,nullptr,"ANKS3",layout()){
 const char*bodyIds[]={"decay","brightness","position","detune","level","on","odd"};
 for(size_t i=0;i<2;++i)for(size_t j=0;j<7;++j)bodyPointers[i][j]=state.getRawParameterValue(juce::String(i==0?"a_":"b_")+bodyIds[j]);
 const char*ids[]={"input","output","mix","balance","cutoff","drive","attack","release","root","filter_on","drive_on","bodies_on","sample","loop","route"};
 for(size_t i=0;i<15;++i)sharedPointers[i]=state.getRawParameterValue(ids[i]);
 const char*seqIds[]={"seq_on","seq_clock","seq_run","seq_bpm","seq_length","seq_division","seq_channel"};for(size_t i=0;i<7;++i)sequencePointers[i]=state.getRawParameterValue(seqIds[i]);
 const char*fields[]={"on","note","velocity","gate"};for(size_t i=0;i<16;++i)for(size_t j=0;j<4;++j)stepPointers[i][j]=state.getRawParameterValue("seq_"+juce::String(int(i+1)).paddedLeft('0',2)+"_"+fields[j]);
 const char*fmIds[]={"fm_on","fm_ratio","fm_index","fm_feedback","fm_mod_attack","fm_mod_decay","fm_mod_sustain","fm_mod_release","fm_velocity","fm_amp_decay","fm_amp_sustain"};for(size_t i=0;i<fmPointers.size();++i)fmPointers[i]=state.getRawParameterValue(fmIds[i]);
 rackEnabled=state.getRawParameterValue("rack_on");rackOrder=state.getRawParameterValue("rack_order");wavePointer=state.getRawParameterValue("waveform");
 for(size_t i=0;i<4;++i){auto prefix="m"+juce::String(int(i+1))+"_";rackTypes[i]=state.getRawParameterValue(prefix+"type");for(size_t j=0;j<anks::rackFields.size();++j)rackPointers[i][j]=state.getRawParameterValue(prefix+anks::rackFields[j].id);}

}
void ResonantLab::prepareToPlay(double sr,int){freePpq=0;audioRate=sr;updateParameters();engine.prepare(sr);sequence.prepare(sr);sequenceStep.store(-1);keyboard.reset();}
juce::AudioProcessorEditor*ResonantLab::createEditor(){return new RackEditor(*this);}
juce::AudioProcessorValueTreeState::ParameterLayout ResonantLab::layout(){
 juce::AudioProcessorValueTreeState::ParameterLayout l;
 auto add=[&](const juce::String&id,const juce::String&name,float a,float b,float step,float def,float skew=1){l.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id,1},name,juce::NormalisableRange<float>(a,b,step,skew),def));};
 auto toggle=[&](const juce::String&id,const juce::String&name,bool def){l.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{id,1},name,def));};
 for(int i=0;i<2;++i){const juce::String prefix=i==0?"a_":"b_",title=i==0?"Corps A ":"Corps B ";
  add(prefix+"decay",title+"resonance",0.05f,8,0.01f,i==0?1.5f:2.5f,0.4f);
  add(prefix+"brightness",title+"brillance",0.05f,1,0.001f,i==0?0.8f:0.55f);
  add(prefix+"position",title+"position",0.02f,0.48f,0.001f,i==0?0.2f:0.31f);
  add(prefix+"detune",title+"accord cents",-1200,1200,0.1f,i==0?0:7);
  add(prefix+"level",title+"niveau dB",-24,12,0.1f,0);
  toggle(prefix+"on",title+"actif",true);toggle(prefix+"odd",title+"impair",false);
 }
 add("input","Entree dB",-24,24,0.1f,0);add("output","Sortie dB",-24,24,0.1f,0);
 add("mix","Source / corps",0,1,0.001f,0.8f);add("balance","Balance A / B",0,1,0.001f,0.5f);
 add("cutoff","Filtre Hz",30,18000,1,6000,0.25f);add("drive","Saturation dB",0,24,0.1f,6);
 add("attack","Attaque s",0,1,0.001f,0,0.3f);add("release","Relachement s",0.01f,4,0.001f,0.3f,0.4f);
 add("root","Note racine MIDI",0,127,1,60);
 toggle("filter_on","Filtre actif",false);toggle("drive_on","Saturation active",false);toggle("bodies_on","Bloc corps actif",true);
 toggle("sample","Utiliser audio importe",false);toggle("loop","Boucle fichier entier",false);
 l.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"route",1},"Trajet",juce::StringArray{"Filtre > Saturation > Corps","Filtre > Corps > Saturation","Saturation > Filtre > Corps","Saturation > Corps > Filtre","Corps > Filtre > Saturation","Corps > Saturation > Filtre"},0));
 l.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"edit",1},"Edition A B lies",juce::StringArray{"A","B","A+B lies"},0));
 toggle("seq_on","Seq actif",false);toggle("seq_run","Seq lecture interne",false);
 l.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"seq_clock",1},"Seq horloge",juce::StringArray{"Hote","Interne"},0));
 add("seq_bpm","Seq tempo interne BPM",30,300,0.1f,120);add("seq_length","Seq longueur",1,16,1,16);
 l.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"seq_division",1},"Seq resolution",juce::StringArray{"1/4","1/8","1/16","1/32"},2));add("seq_channel","Seq canal MIDI",1,16,1,1);
 const int notes[]={36,36,43,36,39,36,46,43,36,48,43,36,39,46,43,34};
 for(int i=0;i<16;++i){juce::String prefix="seq_"+juce::String(i+1).paddedLeft('0',2)+"_",title="Seq "+juce::String(i+1)+" ";toggle(prefix+"on",title+"actif",i%2==0||i==3);add(prefix+"note",title+"note MIDI",0,127,1,float(notes[i]));add(prefix+"velocity",title+"velocite",1,127,1,i%4==0?110.f:85.f);add(prefix+"gate",title+"duree %",5,100,1,60);}
 toggle("rack_on","Architecture rack active",true);
 juce::StringArray orders;for(auto&o:anks::rackOrders())orders.add(juce::String(o[0]+1)+" > "+juce::String(o[1]+1)+" > "+juce::String(o[2]+1)+" > "+juce::String(o[3]+1));
 l.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"rack_order",1},"Ordre instances rack",orders,0));
 l.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"waveform",1},"Onde interne",juce::StringArray{"Impact","Sinus","Triangle","Dent de scie","Carre"},0));
 for(int i=0;i<4;++i){auto prefix="m"+juce::String(i+1)+"_",title="Instance "+juce::String(i+1)+" ";l.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{prefix+"type",1},title+"module",juce::StringArray{"Vide","Double corps","Filtre / EQ","Saturation","Granulaire","Delay","Chorus","Decalage de frequence","Freeze spectral","Semi-modulaire"},i==0?1:0));for(size_t j=0;j<50;++j){auto&f=anks::rackFields[j];add(prefix+f.id,title+f.label,f.min,f.max,f.step,f.initial,f.skew);}}
 for(int i=0;i<4;++i)add("m"+juce::String(i+1)+"_disto_mode","Instance "+juce::String(i+1)+" type disto",0,2,1,0);
 for(int i=0;i<4;++i)for(size_t j=51;j<anks::rackFields.size();++j){auto&f=anks::rackFields[j];add("m"+juce::String(i+1)+"_"+f.id,"Instance "+juce::String(i+1)+" "+f.label,f.min,f.max,f.step,f.initial,f.skew);}

 add("fm_on","Source FM active",0.0f,1.0f,1.0f,0.0f,1.0f);
 add("fm_ratio","FM rapport",0.125f,16.0f,0.001f,2.0f,1.0f);
 add("fm_index","FM intensite",0.0f,12.0f,0.001f,2.0f,1.0f);
 add("fm_feedback","FM feedback",0.0f,0.95f,0.001f,0.0f,1.0f);
 add("fm_mod_attack","FM modulation attaque s",0.001f,2.0f,0.001f,0.005f,0.4f);
 add("fm_mod_decay","FM modulation declin s",0.005f,8.0f,0.001f,0.6f,0.4f);
 add("fm_mod_sustain","FM modulation maintien",0.0f,1.0f,0.001f,0.0f,1.0f);
 add("fm_mod_release","FM modulation relachement s",0.005f,8.0f,0.001f,0.3f,0.4f);
 add("fm_velocity","FM sensibilite velocite",0.0f,1.0f,0.001f,0.5f,1.0f);
 add("fm_amp_decay","FM volume declin s",0.005f,8.0f,0.001f,0.5f,0.4f);
 add("fm_amp_sustain","FM volume maintien",0.0f,1.0f,0.001f,1.0f,1.0f);
 return l;
}
void ResonantLab::updateParameters(){
 readRack();auto&p=engine.parameters;
 auto fm=[&](size_t i){return fmPointers[i]->load();};p.fm.enabled=fm(0)>.5f;p.fm.ratio=fm(1);p.fm.index=fm(2);p.fm.feedback=fm(3);p.fm.modAttack=fm(4);p.fm.modDecay=fm(5);p.fm.modSustain=fm(6);p.fm.modRelease=fm(7);p.fm.velocity=fm(8);p.fm.ampDecay=fm(9);p.fm.ampSustain=fm(10);
 for(size_t i=0;i<2;++i){auto get=[&](size_t j){return bodyPointers[i][j]->load();};auto&b=p.body[i];b.decay=get(0);b.brightness=get(1);b.position=get(2);b.detune=get(3);b.levelDb=get(4);b.enabled=get(5)>0.5f;b.odd=get(6)>0.5f;}
 auto get=[&](size_t j){return sharedPointers[j]->load();};p.inputDb=get(0);p.outputDb=get(1);p.bodyMix=get(2);p.balance=get(3);p.cutoff=get(4);p.driveDb=get(5);p.attack=get(6);p.release=get(7);p.root=get(8);p.filterOn=get(9)>0.5f;p.driveOn=get(10)>0.5f;p.bodiesOn=get(11)>0.5f;p.useSample=get(12)>0.5f;p.loop=get(13)>0.5f;p.route=int(get(14));
}
anks::SequenceConfig ResonantLab::readSequence()const{
 anks::SequenceConfig s;s.enabled=sequencePointers[0]->load()>0.5f;s.internal=sequencePointers[1]->load()>0.5f;s.run=sequencePointers[2]->load()>0.5f;s.bpm=sequencePointers[3]->load();s.length=int(sequencePointers[4]->load());s.division=int(sequencePointers[5]->load());s.channel=int(sequencePointers[6]->load());
 for(size_t i=0;i<16;++i){auto&step=s.steps[i];step.enabled=stepPointers[i][0]->load()>0.5f;step.note=int(stepPointers[i][1]->load());step.velocity=int(stepPointers[i][2]->load());step.gate=stepPointers[i][3]->load()*0.01;}
 return s;
}
void ResonantLab::editBodyParameter(size_t index,float value){
 if(index>5)return;
 const char*ids[]={"decay","brightness","position","detune","level","odd"};int mode=int(state.getRawParameterValue("edit")->load());
 for(int b=0;b<2;++b)if(mode==2||mode==b){auto id=juce::String(b==0?"a_":"b_")+ids[index];auto*p=state.getParameter(id);p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(value));p->endChangeGesture();}
}
void ResonantLab::setRouteOrder(const std::array<int,3>&order){for(size_t i=0;i<anks::routes.size();++i)if(anks::routes[i]==order){auto*p=state.getParameter("route");p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(float(i)));p->endChangeGesture();return;}}
void ResonantLab::processBlock(juce::AudioBuffer<float>&out,juce::MidiBuffer&midi){
 juce::ScopedNoDenormals guard;out.clear();
 // Imports and restores never wait on the audio thread: a contended block is silent.
 const juce::ScopedTryLock lock(getCallbackLock());if(!lock.isLocked()){droppedBlock=true;midi.clear();return;}
 if(droppedBlock){engine.panic();sequence.reset();droppedBlock=false;}
 updateParameters();
 auto config=readSequence();anks::HostClock host;
 if(auto*playHead=getPlayHead())if(auto pos=playHead->getPosition()){auto bpm=pos->getBpm(),ppq=pos->getPpqPosition();if(bpm&&ppq&&std::isfinite(*bpm)&&*bpm>0&&std::isfinite(*ppq)){host.valid=true;host.bpm=*bpm;host.ppq=*ppq;host.playing=pos->getIsPlaying();}}
 currentBpm.store(float(config.internal?config.bpm:(host.valid?host.bpm:config.bpm)));
 clockStatus.store(!config.enabled?0:(config.internal?(config.run?2:1):(!host.valid?3:(host.playing?5:4))));sequence.beginBlock(config,host);
 engine.parameters.rack.clockRunning=!host.valid||host.playing;engine.parameters.rack.bpm=currentBpm.load();engine.parameters.rack.ppq=host.valid?host.ppq:freePpq;freePpq+=double(out.getNumSamples())*currentBpm.load()/(60*audioRate);engine.beginBlock();
 keyboard.processNextMidiBuffer(midi,0,out.getNumSamples(),true);int cursor=0;
 for(const auto meta:midi){const int pos=juce::jlimit(cursor,out.getNumSamples(),meta.samplePosition);render(out,cursor,pos);handle(meta.getMessage());cursor=pos;}
 render(out,cursor,out.getNumSamples());midi.clear();sequenceStep.store(sequence.currentStep());
 float candidate=float(engine.peak),previous=meterPeak.load();while(previous<candidate&&!meterPeak.compare_exchange_weak(previous,candidate)){}if(engine.clipped)clipLatch.store(true);
}
void ResonantLab::handle(const juce::MidiMessage&m){
 const int ch=m.getChannel();if(ch<1||ch>16)return;
 if(m.isNoteOn())engine.noteOn(m.getNoteNumber(),ch,m.getFloatVelocity());
 else if(m.isNoteOff())engine.noteOff(m.getNoteNumber(),ch);
 else if(m.isPitchWheel())engine.pitchBend(ch,(m.getPitchWheelValue()-8192)/8192.0*2);
 else if(m.isController())engine.control(ch,m.getControllerNumber(),m.getControllerValue());
}
void ResonantLab::render(juce::AudioBuffer<float>&out,int start,int end){for(int i=start;i<end;++i){auto events=sequence.tick();for(int j=0;j<events.count;++j){const auto&e=events.items[size_t(j)];if(e.on)engine.noteOn(e.note,e.channel,e.velocity,1);else engine.noteOff(e.note,e.channel,1);}const auto y=engine.nextStereo();out.setSample(0,i,float(y.left));out.setSample(1,i,float(y.right));}}

juce::String ResonantLab::decode(const juce::MemoryBlock& b,std::unique_ptr<anks::Sample>& s){return anks::decodeSample(b,s);}
juce::String ResonantLab::loadSample(const juce::File&file){
 if(!file.existsAsFile())return "Fichier absent.";
 if(file.getSize()>16*1024*1024)return "Fichier superieur a 16 Mo.";
 juce::MemoryBlock bytes;if(!file.loadFileAsData(bytes))return "Lecture impossible.";
 return loadSampleBytes(std::move(bytes),file.getFileName());
}
juce::String ResonantLab::loadSampleBytes(juce::MemoryBlock bytes,const juce::String&name){
 std::unique_ptr<anks::Sample>s;auto error=decode(bytes,s);if(error.isNotEmpty())return error;
 {const juce::ScopedLock lock(getCallbackLock());const juce::ScopedLock metadataLock(sampleStateLock);engine.panic();sequence.reset();source=std::move(s);engine.sample=source.get();sourceBytes=std::move(bytes);sourceName=name;}
 state.getParameter("fm_on")->setValueNotifyingHost(0);state.getParameter("sample")->setValueNotifyingHost(1);return {};
}
juce::String ResonantLab::loadFactory(int index){int size=0;auto*data=anks::factory::bytes(index,size);if(!data||size<=0)return "Excitation interne absente.";auto error=loadSampleBytes(juce::MemoryBlock(data,size),anks::factory::name(index));if(error.isEmpty()){auto*p=state.getParameter("root");p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(float(anks::factory::root(index))));p->endChangeGesture();}return error;}
juce::String ResonantLab::sampleLabel(){const juce::ScopedLock lock(sampleStateLock);return sourceName;}
void ResonantLab::getStateInformation(juce::MemoryBlock&dest){
 auto snapshot=state.copyState();snapshot.setProperty("anksSuiteSchema",3,nullptr);snapshot.setProperty("instrumentVersion","1.3.0-alpha.2",nullptr);{const juce::ScopedLock lock(sampleStateLock);snapshot.setProperty("sourceName",sourceName,nullptr);snapshot.setProperty("audioBase64",sourceBytes.toBase64Encoding(),nullptr);}
 auto xml=snapshot.createXml();copyXmlToBinary(*xml,dest);
}
void ResonantLab::setStateInformation(const void*data,int size){restoreState(data,size);}
bool ResonantLab::restoreState(const void*data,int size){
 // Limit allocations from corrupt host state before parsing.
 if(size<=0||size>24*1024*1024)return false;
 auto xml=getXmlFromBinary(data,size);if(!xml||!xml->hasTagName("ANKS3"))return false;
 auto tree=juce::ValueTree::fromXml(*xml);juce::MemoryBlock bytes;std::unique_ptr<anks::Sample>s;
 const auto encoded=tree.getProperty("audioBase64").toString();
 if(encoded.isNotEmpty()&&(!bytes.fromBase64Encoding(encoded)||decode(bytes,s).isNotEmpty()))return false;
 // Append missing new parameters; a legacy state always retains the classic motor.
 for(auto*param:getParameters())if(auto*ranged=dynamic_cast<juce::RangedAudioParameter*>(param))if(!tree.getChildWithProperty("id",ranged->paramID).isValid()){juce::ValueTree v("PARAM");v.setProperty("id",ranged->paramID,nullptr);v.setProperty("value",ranged->paramID=="rack_on"?0.f:ranged->convertFrom0to1(ranged->getDefaultValue()),nullptr);tree.addChild(v,-1,nullptr);}
 const juce::ScopedLock lock(getCallbackLock());const juce::ScopedLock metadataLock(sampleStateLock);engine.panic();sequence.reset();source=std::move(s);engine.sample=source.get();freePpq=0;sourceBytes=std::move(bytes);sourceName=tree.getProperty("sourceName","Impact synthetique interne").toString();tree.removeProperty("audioBase64",nullptr);tree.removeProperty("sourceName",nullptr);state.replaceState(tree);return true;
}
bool ResonantLab::savePreset(const juce::File&file){juce::MemoryBlock data;getStateInformation(data);return file.replaceWithData(data.getData(),data.getSize());}
bool ResonantLab::loadPreset(const juce::File&file){if(file.getSize()>24*1024*1024)return false;juce::MemoryBlock data;if(!file.loadFileAsData(data))return false;return restoreState(data.getData(),int(data.getSize()));}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new ResonantLab;}

void ResonantLab::readRack(){
 auto&r=engine.parameters.rack;r.enabled=rackEnabled->load()>.5f;r.order=int(rackOrder->load());engine.parameters.waveform=int(wavePointer->load());
 for(size_t i=0;i<4;++i){auto&p=r.modules[i];p.type=int(rackTypes[i]->load());auto v=[&](size_t n){return rackPointers[i][n]->load();};
  p.on=v(0)>.5f;
  p.mix=v(1);
  p.level=v(2);
  p.balance=v(3);
  p.body[0].decay=v(5);
  p.body[0].brightness=v(6);
  p.body[0].position=v(7);
  p.body[0].detune=v(8);
  p.body[0].level=v(9);
  p.body[0].on=v(10)>.5f;
  p.body[0].odd=v(11)>.5f;
  p.body[1].decay=v(12);
  p.body[1].brightness=v(13);
  p.body[1].position=v(14);
  p.body[1].detune=v(15);
  p.body[1].level=v(16);
  p.body[1].on=v(17)>.5f;
  p.body[1].odd=v(18)>.5f;
  p.filterMode=int(v(19));
  p.cutoff=v(20);
  p.q=v(21);
  p.lfoRate=v(22);
  p.lfoDepth=v(23);
  p.lfoShape=int(v(24));
  p.lfoSync=v(25)>.5f;
  p.lfoBeats=v(26);
  p.eqFrequency[0]=v(27);
  p.eqGain[0]=v(28);
  p.eqQ[0]=v(29);
  p.eqFrequency[1]=v(30);
  p.eqGain[1]=v(31);
  p.eqQ[1]=v(32);
  p.eqFrequency[2]=v(33);
  p.eqGain[2]=v(34);
  p.eqQ[2]=v(35);
  p.eqFrequency[3]=v(36);
  p.eqGain[3]=v(37);
  p.eqQ[3]=v(38);
  p.drive=v(39);
  p.tone=v(40);
  p.grainSize=v(41);
  p.density=v(42);
  p.position=v(43);
  p.scatter=v(44);
  p.pitch=v(45);
  p.time=v(46);
  p.feedback=v(47);
  p.rate=v(48);
  p.depth=v(49);p.distoMode=int(v(50));
  p.shiftHz=v(51);p.freezeHold=v(52)>.5f;
  p.smCutoff=v(53);p.smResonance=v(54);p.smDrive=v(55);p.smHighpass=v(56)>.5f;
  p.smAttack=v(57);p.smDecay=v(58);p.smSustain=v(59);p.smRelease=v(60);p.smDrone=v(61)>.5f;p.smLfoRate=v(62);p.smLfoShape=v(63)>.5f;
  for(size_t r=0;r<2;++r){p.smSource[r]=int(v(64+r*3));p.smDestination[r]=int(v(65+r*3));p.smAmount[r]=v(66+r*3);}
 }
}

void ResonantLab::swapRackSlots(int from,int to){
 if(from<0||to<0||from>3||to>3||from==to)return;auto order=anks::rackOrders()[size_t(juce::jlimit(0,23,int(rackOrder->load())))];std::swap(order[size_t(from)],order[size_t(to)]);for(size_t i=0;i<24;++i)if(anks::rackOrders()[i]==order){auto*p=state.getParameter("rack_order");p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(float(i)));p->endChangeGesture();return;}
}
void ResonantLab::convertLegacy(){
 auto put=[&](juce::String id,float v){auto*p=state.getParameter(id);p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(v));p->endChangeGesture();};
 for(int i=0;i<4;++i)for(auto&f:anks::rackFields)put("m"+juce::String(i+1)+"_"+f.id,f.initial);
 for(int i=0;i<4;++i)put("m"+juce::String(i+1)+"_type",float(i==0?1:i==1?2:i==2?3:0));
 const char*fields[]={"decay","brightness","position","detune","level","on","odd"};for(int b=0;b<2;++b)for(auto*f:fields)put("m1_b"+juce::String(b+1)+"_"+f,value((juce::String(b==0?"a_":"b_")+f).toRawUTF8()));
 put("m1_mix",value("mix"));put("m1_balance",value("balance"));put("m1_on",value("bodies_on"));put("m2_cutoff",value("cutoff"));put("m2_mix",1);put("m2_on",value("filter_on"));put("m3_drive",value("drive"));put("m3_tone",18000);put("m3_mix",1);put("m3_on",value("drive_on"));
 auto old=anks::routes[size_t(juce::jlimit(0,5,int(value("route"))))];std::array<int,4>order{{(old[0]+1)%3,(old[1]+1)%3,(old[2]+1)%3,3}};for(size_t i=0;i<24;++i)if(anks::rackOrders()[i]==order)put("rack_order",float(i));put("rack_on",1);
}
bool ResonantLab::saveRecipe(const juce::File&file,bool module,int identity){
 if(identity<0||identity>3)return false;auto tree=state.copyState();juce::XmlElement xml("ANKS_RECIPE");xml.setAttribute("schema",3);xml.setAttribute("kind",module?"module":"chain");auto prefix="m"+juce::String(identity+1)+"_";
 for(int i=0;i<tree.getNumChildren();++i){auto c=tree.getChild(i);auto id=c.getProperty("id").toString();if((module&&id.startsWith(prefix))||(!module&&(id.startsWith("m1_")||id.startsWith("m2_")||id.startsWith("m3_")||id.startsWith("m4_")||id=="rack_order"||id=="input"||id=="output"))){auto*e=xml.createNewChildElement("PARAM");e->setAttribute("id",module?id.substring(prefix.length()):id);e->setAttribute("value",double(c.getProperty("value")));}}
 return file.replaceWithText(xml.toString());
}
bool ResonantLab::loadRecipe(const juce::File&file,bool module,int identity){
 if(identity<0||identity>3||file.getSize()>1024*1024)return false;auto xml=juce::XmlDocument::parse(file);if(!xml||!xml->hasTagName("ANKS_RECIPE")||(xml->getIntAttribute("schema")<1||xml->getIntAttribute("schema")>3)||xml->getStringAttribute("kind")!=(module?"module":"chain"))return false;
 std::vector<std::pair<juce::RangedAudioParameter*,float>>pending;juce::StringArray seen;auto prefix="m"+juce::String(identity+1)+"_";
 for(auto*e:xml->getChildIterator()){auto key=e->getStringAttribute("id");if(seen.contains(key))return false;seen.add(key);auto id=module?prefix+key:key;
 bool allowed=module?id.startsWith(prefix):(id.startsWith("m1_")||id.startsWith("m2_")||id.startsWith("m3_")||id.startsWith("m4_")||id=="rack_order"||id=="input"||id=="output");auto*p=state.getParameter(id);double v=e->getDoubleAttribute("value",std::numeric_limits<double>::quiet_NaN());if(!allowed||!p||!std::isfinite(v)||v<p->getNormalisableRange().start||v>p->getNormalisableRange().end)return false;pending.emplace_back(p,float(v));}
 // Append new controls at their defaults when loading earlier recipe schemas.
 if(xml->getIntAttribute("schema")<3){size_t first=xml->getIntAttribute("schema")==1?50:51;for(int i=0;i<(module?1:4);++i)for(size_t j=first;j<anks::rackFields.size();++j){auto&f=anks::rackFields[j];auto key=module?juce::String(f.id):"m"+juce::String(i+1)+"_"+f.id;if(!seen.contains(key))pending.emplace_back(state.getParameter(module?prefix+key:key),f.initial);}}
 const int required=module?int(anks::rackFields.size()+1):int(4*(anks::rackFields.size()+1)+3);if(int(pending.size())!=required)return false;
 const juce::ScopedLock lock(getCallbackLock());engine.panic();sequence.reset();for(auto&entry:pending){entry.first->beginChangeGesture();entry.first->setValueNotifyingHost(entry.first->convertTo0to1(entry.second));entry.first->endChangeGesture();}state.getParameter("rack_on")->setValueNotifyingHost(1);return true;
}
