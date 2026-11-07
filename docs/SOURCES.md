# Registre documentaire — 2 novembre 2026

Les documents constructeur sont des sources primaires même lorsqu’ils sont hébergés par une archive. Les analyses indépendantes et les brevets ne sont pas assimilés à un manuel d’implémentation du WSA1.

| ID | Document | Accès pendant cette étude | Utilité / réserve |
|---|---|---|---|
| S1 | [Technics Basic Functions](https://www.synth-db.com/Synths/Technics/sx-WSA1/sx-WSA1-OM.pdf) | Sommaire et extraits indexés; ouverture/téléchargement échoués | Présentation constructeur; pas considéré comme intégralement lu |
| S2 | [Technics Practical Applications](https://manualzz.com/doc/13504780/technics-sx-wsa1r-manual) | Texte du manuel consulté, surtout édition/modélisation; OCR imparfait | Source des commandes et de l’organisation musicale; pp.18–37 prioritaires |
| S3 | [Service Manual EMID951601, 46 pages](https://www.manualslib.com/manual/4569712/Technics-Sx-Wsa1.html) | Couverture, spécifications, schéma-bloc et index consultés | Ne livre pas les équations internes des puces |
| S3a | [Schéma-bloc, première moitié](https://www.manualslib.com/manual/4569712/Technics-Sx-Wsa1.html?page=17) | Texte consulté | Partie commande; OCR des liaisons insuffisant |
| S3b | [Schéma-bloc, seconde moitié](https://www.manualslib.com/manual/4569712/Technics-Sx-Wsa1.html?page=18) | Texte consulté | Modeling LSI, Tone Generator, Wave ROM, DSP, sortie |
| S4 | [Reference Guide 2, 56 pages](https://www.manualslib.com/manual/4315769/Technics-Sx-Wsa1.html) | Sommaire et sections ciblées consultés | Effets et données MIDI; pas une transcription exhaustive |
| S4a | [MIDI, p.34](https://www.manualslib.com/manual/4315769/Technics-Sx-Wsa1.html?page=34) | Texte consulté | Répartition de colonnes OCR à vérifier visuellement |
| S4b | [Flux MIDI, p.37](https://www.manualslib.com/manual/4315769/Technics-Sx-Wsa1.html?page=37) | Texte consulté | Entrées/sorties, parties, filtrage |
| S4c | [MODEL DATA, p.51](https://www.manualslib.com/manual/4315769/Technics-Sx-Wsa1.html?page=51) | Texte + scan de la page examinés | Plages de données; champs réservés à conserver distincts |
| S5 | Reference Guide 1, annoncé 20 pages | Repéré dans des catalogues; PDF non récupéré | Inventaires des sons et sources à compléter |
| S6 | [JP2534636B2 — Music synthesizer](https://patents.google.com/patent/JP2534636B2/en) | Description consultée, passages sur excitation, retards et couplage | Brevet Matsushita; aucun lien produit démontré |
| S7 | [JPH05188950A](https://patents.google.com/patent/JPH05188950A/en) | Résumé/description initiale consultés | Résonances de piano; lien WSA1 non établi |
| S8 | [JPH06214563A](https://patents.google.com/patent/JPH06214563A/en) | Résumé/description initiale consultés | Réseaux de résonance et spatialisation; lien produit non établi |
| S9 | [Analyse L7A1429](https://arqueologiadigital.org/KN5000-docs/wsa1-modeling-lsi/) | Contenu indexé consulté; accès direct échoué | Recherche indépendante à auditer, pas une spécification constructeur |
| S9a | [Projet de désassemblage](https://arqueologiadigital.org/KN5000-docs/wsa1-disassembly/) | Présentation indexée consultée | Pas de dépôt ou de commit audité dans cette session |
| S10 | [Thor Zollinger — Dissecting the Technics WSA1](https://javelinart.com/dissecting-wsa1.html) | Article consulté | Expériences spectrales; hypothèses explicites de l’auteur |
| S11 | [Julius O. Smith — Digital Waveguide Models](https://www.dsprelated.com/freebooks/pasp/Digital_Waveguide_Models.html) | Chapitre consulté | Théorie de notre banc; pas preuve du DSP Technics |
| S11a | [Frequency-Dependent Damping](https://www.dsprelated.com/freebooks/pasp/Frequency_Dependent_Damping.html) | Consulté | Pertes et passivité des boucles |
| S12 | [Exemple CMake officiel JUCE](https://github.com/juce-framework/JUCE/blob/master/examples/CMake/AudioPlugin/CMakeLists.txt) | Consulté | Préparation de l’adaptateur VST3 |
| S12a | [Tutoriel plugin JUCE](https://juce.com/tutorials/tutorial_code_basic_plugin/) | Repéré et extrait consulté | Référence MIDI/audio |

## Pistes repérées, non retenues comme preuves techniques

- [Index historique de manuels WSA1](http://wsa1.technicskeyboards.com/wsa1-manuals/) : inaccessible pendant cette recherche.
- [Discussion signalant le Technical Guide](https://www.sequencer.de/synthesizer/threads/technics-wsa1-handbuch.122835/) : atteste un signalement, pas le contenu du guide.
- [Sound Programming : liens de manuels](https://soundprogramming.net/synthesizers/technics/technics-sx-wsa1/) : les liens PDF essayés redirigent vers l’accueil.
- [Guide pédagogique Brassee](https://www.brassee.com/files/002_The_Technics_WSA1_and_Physical_Modeling.pdf) : extraits indexés repérés; téléchargement non valide, pas inclus.
- [Démonstration commentée du moteur](https://www.matrixsynth.com/2019/07/technics-wsa1-closer-look-at-modelling.html) : repérée; vidéo non visionnée et non utilisée pour mesurer le moteur.
- [Brevet US20020055840A1](https://patents.google.com/patent/US20020055840A1/en) : priorité postérieure à la machine; exclu de sa généalogie démontrée.

## Suite de collecte

1. Récupérer les guides complets dans une archive accessible et vérifier titres, éditions et nombre de pages.
2. Relever le catalogue DRIVER/RESONATOR sans déduire le nombre d’algorithmes du nombre de noms.
3. Transcrire les tables SysEx avec vérification des images, tailles, offsets et encodages.
4. Vérifier les puces et horloges sur schémas lisibles; ne pas déduire directement une fréquence audio d’un oscillateur.
5. Obtenir une machine ou un corpus contrôlé pour les essais; aucune possession de WSA1 n’est supposée.

Les originaux restent chez leurs hébergeurs. Le dossier contient notre synthèse et notre code, sans copie des manuels, du firmware ou de la banque sonore Technics.
