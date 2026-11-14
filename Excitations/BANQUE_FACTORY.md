# Banque factory : 24 ajouts originaux / 36 sources au total

Sources synthétiques ANKS, générateur reproductible : `tools/generate_factory_bank.py` (NumPy). Aucune source externe. WAV mono PCM16 / 48 kHz, crête −6 dBFS environ. Les racines des textures non accordées sont des repères de transposition. Les sons sont intégrés dans le VST3 et le Standalone; aucun WAV à installer.

Attaques, partiels, bruit et variations temporelles fournissent de la matière au résonateur et aux grains. Ces sons ne sont pas des boucles parfaitement jointives. La durée audible reste contrôlée par la note, l’enveloppe et les queues du rack.

| Famille | Excitation | Racine MIDI | Durée |
|---|---|---:|---:|
| Bois et cordes | Bois_Creux.wav | 48 | 1.6 s |
| Bois et cordes | Corde_Pincee.wav | 45 | 1.6 s |
| Bois et cordes | Kalimba_Douce.wav | 60 | 1.6 s |
| Bois et cordes | Marimba_Fibre.wav | 48 | 1.6 s |
| Bois et cordes | Corde_Frottee.wav | 55 | 1.6 s |
| Bois et cordes | Bambou_Pulse.wav | 60 | 1.6 s |
| Metaux | Cloche_Felee.wav | 60 | 2.0 s |
| Metaux | Plaque_Vibrante.wav | 48 | 2.0 s |
| Metaux | Ressort_Metallique.wav | 55 | 2.0 s |
| Metaux | Bol_Chantant.wav | 57 | 2.0 s |
| Metaux | Carillon_Mobile.wav | 72 | 2.0 s |
| Metaux | Lamelle_Rouillee.wav | 45 | 2.0 s |
| Textures organiques | Papier_Froisse.wav | 60 | 2.2 s |
| Textures organiques | Sable_Roulant.wav | 60 | 2.2 s |
| Textures organiques | Brosse_Granuleuse.wav | 60 | 2.2 s |
| Textures organiques | Gouttes_Resonantes.wav | 60 | 2.2 s |
| Textures organiques | Souffle_Anche.wav | 50 | 2.2 s |
| Textures organiques | Crepitement_Doux.wav | 60 | 2.2 s |
| Matieres numeriques | FM_Evolutive.wav | 36 | 2.4 s |
| Matieres numeriques | Impulsions_Irisees.wav | 60 | 2.4 s |
| Matieres numeriques | Spectre_Mouvant.wav | 48 | 2.4 s |
| Matieres numeriques | Voyelle_Morphing.wav | 45 | 2.4 s |
| Matieres numeriques | Pixels_Fondants.wav | 60 | 2.4 s |
| Matieres numeriques | Nuage_Harmonique.wav | 55 | 2.4 s |

Une seule excitation est chargée à la fois. Agrandir la banque augmente surtout la taille du plugin; cela ne multiplie pas les modules DSP joués simultanément.
