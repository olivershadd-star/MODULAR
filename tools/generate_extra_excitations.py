"""Generate eight original synthetic excitation WAVs; no third-party samples."""
from pathlib import Path
import wave
import numpy as np
folder=Path(__file__).resolve().parents[1]/'Excitations'
sr=48000
rng=np.random.default_rng(3031995)
rows=[]
for name,duration,root in [('Kick_Pulse',0.55,35),('Snare_Poudre',0.35,60),('Clap_Triplet',0.3,60),('Metal_Cluster',1.4,60),('FM_C2',1.5,36),('Grain_Numerique',0.9,60),('Voyelle_Synthetique',1.2,45),('Verre_Aigu',1.0,72)]:
 t=np.arange(int(sr*duration))/sr
 noise=rng.uniform(-1,1,len(t))
 if name=='Kick_Pulse':
  f=440*2**((35-69)/12);phase=2*np.pi*(f*t+130*0.018*(1-np.exp(-t/0.018)));x=np.sin(phase)*np.exp(-t/0.15)+noise*np.exp(-t/0.004)*0.2
 elif name=='Snare_Poudre':x=noise*np.exp(-t/0.055)+0.3*np.sin(2*np.pi*185*t)*np.exp(-t/0.07)
 elif name=='Clap_Triplet':x=noise*sum(np.where(t>=a,np.exp(-np.maximum(0,t-a)/d),0) for a,d in [(0,.008),(.014,.008),(.029,.008),(.048,.06)])
 elif name=='Metal_Cluster':x=sum(np.sin(2*np.pi*f*t)*np.exp(-t/(0.1+i*0.06))/(i+1)**0.6 for i,f in enumerate([270,731,1451,2093,3503]))
 elif name=='FM_C2':
  f=440*2**((36-69)/12);x=np.sin(2*np.pi*f*t+(1+7*np.exp(-t/.2))*np.sin(2*np.pi*f*2.01*t))*np.exp(-t/.4)
 elif name=='Grain_Numerique':x=np.round(noise*3)/3*(0.15+0.85*np.abs(np.sin(2*np.pi*39*t))**9)*np.exp(-t/.6)
 elif name=='Voyelle_Synthetique':
  x=np.zeros_like(t)
  for k in range(1,61):
   f=k*110;weight=sum(np.exp(-.5*((f-c)/w)**2) for c,w in [(500,90),(1400,180),(2500,250)]);x+=weight*np.sin(2*np.pi*f*t)/k**.4
  x*=np.sin(np.pi*np.minimum(t/.1,1)/2)*np.exp(-t/2)
 else:
  f=440*2**((72-69)/12);x=sum(np.sin(2*np.pi*f*r*t)*np.exp(-t/(.15+i*.06))/(i+1) for i,r in enumerate([1,2.71,4.31,6.93]))
 fade=min(64,len(x)//4);x[:fade]*=np.linspace(0,1,fade);x[-fade:]*=np.linspace(1,0,fade);x=x/max(np.max(np.abs(x)),1e-12)*.5
 with wave.open(str(folder/(name+'.wav')),'wb') as w:
  w.setnchannels(1);w.setsampwidth(2);w.setframerate(sr);w.writeframes((x*32767).astype('<i2').tobytes())
 rows.append((name+'.wav',root,duration))
(folder/'NOUVELLES_EXCITATIONS.md').write_text('# Excitations additionnelles originales ANKS\n\nWAV mono PCM16 / 48 kHz, crete environ -6 dBFS. Racine proposee : point de depart de transposition, les bruits n’ont pas de fondamentale imposee.\n\n| Fichier | Racine MIDI | Duree s |\n|---|---:|---:|\n'+''.join(f'| {n} | {r} | {d} |\n' for n,r,d in rows))
print('Generated eight original excitations')
