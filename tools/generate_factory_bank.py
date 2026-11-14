"""Original ANKS synthesis. Deterministic PCM16/48 kHz factory extension.

Only creates the 24 named additions; never rewrites the twelve original WAVs.
Requires NumPy. Run from any directory with Python 3.
"""
from pathlib import Path
import json
import wave
import numpy as np

ROOT = Path(__file__).resolve().parents[1]
SR = 48000
FAMILIES = [
    ('Bois et cordes', [('Bois_Creux', 48), ('Corde_Pincee', 45),
      ('Kalimba_Douce', 60), ('Marimba_Fibre', 48), ('Corde_Frottee', 55), ('Bambou_Pulse', 60)]),
    ('Metaux', [('Cloche_Felee', 60), ('Plaque_Vibrante', 48),
      ('Ressort_Metallique', 55), ('Bol_Chantant', 57), ('Carillon_Mobile', 72), ('Lamelle_Rouillee', 45)]),
    ('Textures organiques', [('Papier_Froisse', 60), ('Sable_Roulant', 60),
      ('Brosse_Granuleuse', 60), ('Gouttes_Resonantes', 60), ('Souffle_Anche', 50), ('Crepitement_Doux', 60)]),
    ('Matieres numeriques', [('FM_Evolutive', 36), ('Impulsions_Irisees', 60),
      ('Spectre_Mouvant', 48), ('Voyelle_Morphing', 45), ('Pixels_Fondants', 60), ('Nuage_Harmonique', 55)]),
]

def make(family, variant, root):
    duration = [1.6, 2.0, 2.2, 2.4][family]
    t = np.arange(round(SR * duration)) / SR
    rng = np.random.default_rng(120300 + family * 100 + variant)
    noise = rng.uniform(-1, 1, t.size)
    smooth = np.convolve(noise, np.ones(17) / 17, mode='same')
    f = 440 * 2 ** ((root - 69) / 12)
    if family == 0:
        ratios = ([1, 2.76, 5.4, 8.93] if variant in (0, 3, 5) else [1, 2, 3, 4, 5, 7])
        x = sum(np.sin(2*np.pi*f*r*t + .08*np.sin(2*np.pi*(1.3+k*.2)*t))
                * np.exp(-t/(.23 + .15*variant)/(1+k*.25))/(k+1)**1.1
                for k, r in enumerate(ratios))
        x += .3*(noise-smooth)*np.exp(-t/.018)
        if variant == 4:
            x *= 1-np.exp(-t/.06)
            x += smooth*.7*(1-np.exp(-t/.02))*np.exp(-t/.8)*(1+.6*np.sin(2*np.pi*17*t))
        if variant == 5:
            x += .35*np.sin(2*np.pi*f*t)*(np.maximum(0, np.sin(2*np.pi*11*t))**6)*np.exp(-t/.7)
    elif family == 1:
        ratios = [1, 2.013 + variant*.09, 2.71, 4.31, 5.83, 8.17, 11.2]
        x = sum(np.sin(2*np.pi*f*r*t + (.3 + .08*k)*np.sin(2*np.pi*(.8+variant*.3)*t))
                * np.exp(-t/(.35 + .17*variant)/(1+k*.16))/(k+1)**.9
                for k, r in enumerate(ratios))
        x += .25*noise*np.exp(-t/.012)
        if variant == 2:
            x += .35*np.sin(2*np.pi*f*t + 9*np.exp(-t/.25)*np.sin(2*np.pi*f*1.41*t))*np.exp(-t/.6)
        if variant == 4:
            for n, r in enumerate([1, 1.25, 1.5, 2]):
                u=np.maximum(0,t-n*.13)
                x += .45*(t>=n*.13)*np.sin(2*np.pi*f*r*u)*np.exp(-u/.5)
    elif family == 2:
        pulse = np.zeros(t.size)
        for onset in np.sort(rng.uniform(.001, duration*.82, 12+variant*9)):
            u=np.maximum(0,t-onset)
            pulse += (t>=onset)*np.exp(-u/(.009+variant*.006))
        x = (noise*.3 + smooth*2)*pulse*np.exp(-t/2)
        if variant == 3:
            x *= .2
            for n in range(9):
                u=np.maximum(0,t-n*.19)
                x += (t>=n*.19)*.5*np.sin(2*np.pi*(510+n*83)*u-35*np.exp(-u/.02))*np.exp(-u/.055)
        if variant == 4:
            x = sum(np.sin(2*np.pi*f*k*t)*(1+.2*np.sin(2*np.pi*(2+k*.17)*t))/k**1.3
                    for k in range(1, 18))*(1-np.exp(-t/.04))*np.exp(-t/.9) + smooth*.7
        x += .2*noise*np.exp(-t/.006)
    else:
        if variant == 0:
            x = np.sin(2*np.pi*f*t+(2+5*np.exp(-t/.5))*np.sin(2*np.pi*f*2.01*t))
        elif variant == 1:
            env = np.maximum(0,np.sin(2*np.pi*(19*t+3*t*t)))**16
            x = env*(noise*.45 + np.sin(2*np.pi*740*t+.8*np.sin(2*np.pi*3*t))*.5)
        elif variant == 2:
            x = sum(np.sin(2*np.pi*f*k*t+.4*np.sin(2*np.pi*(.4+k*.08)*t))
                    *(0.3+0.7*np.sin(np.pi*(t/duration+k/12))**2)/k
                    for k in range(1, 25))
        elif variant == 3:
            x = np.zeros(t.size)
            for k in range(1, 65):
                freq=f*k
                weight = sum(np.exp(-.5*((freq-(a+(b-a)*t/duration))/w)**2)
                             for a,b,w in [(450,800,100),(1400,1050,160),(2500,3000,220)])
                x += weight*np.sin(2*np.pi*freq*t)/k**.55
        elif variant == 4:
            held = np.repeat(rng.uniform(-1,1,(t.size+11)//12),12)[:t.size]
            x = np.convolve(held,np.ones(13)/13,mode='same')*(.35+.65*np.sin(2*np.pi*23*t)**8)
        else:
            x = sum(np.sin(2*np.pi*f*r*t+.6*np.sin(2*np.pi*(.3+k*.13)*t))/(k+1)
                    for k,r in enumerate([1,1.5,2,3,4,6,8]))
        x *= (1-np.exp(-t/.008))*np.exp(-t/1.1)
        x += .18*noise*np.exp(-t/.009)
    x -= x.mean()
    # Gentle endpoint fades retain the attack and avoid file-boundary clicks.
    attack = 48
    release = 1920
    x[:attack] *= np.sin(np.linspace(0,np.pi/2,attack))**2
    x[-release:] *= np.sin(np.linspace(np.pi/2,0,release))**2
    x *= .5 / max(float(np.max(np.abs(x))), 1e-12)
    return np.rint(x*32767).astype('<i2')

def main():
    rows=[]
    for family,(category,items) in enumerate(FAMILIES):
        for variant,(name,root) in enumerate(items):
            pcm=make(family,variant,root)
            path=ROOT/'Excitations'/(name+'.wav')
            with wave.open(str(path),'wb') as w:
                w.setnchannels(1);w.setsampwidth(2);w.setframerate(SR);w.writeframes(pcm.tobytes())
            rows.append(dict(file=path.name,category=category,root=root,seconds=len(pcm)/SR))
    (ROOT/'Excitations'/'factory_extension.json').write_text(json.dumps(rows,ensure_ascii=False,indent=2)+'\n')
    print(f'Generated {len(rows)} original excitations; originals unchanged.')

if __name__=='__main__':
    main()
