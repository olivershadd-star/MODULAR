# M-VAVE FM‑1 — recherche pour Modular, 22 novembre 2026

## Sources primaires consultées

- [FM1 Open officiel](https://github.com/m-vave/fm1_open), README et [notices](https://github.com/m-vave/fm1_open/blob/main/THIRD_PARTY.md) : application GPLv3 avec exception de liaison SDK. Le moteur natif propriétaire est une bibliothèque binaire pour le processeur JieLi ; sa permission vise les appareils FM1. Ce binaire n’est pas un moteur VST Windows réutilisable directement. Certains composants du moteur web ont leurs propres sources/licences.
- [fm1-dock](https://github.com/joakimeriksson/fm1-dock), README : catalogue communautaire de synthèses, architecture de modules pour le matériel, GPL‑3.0. Les tailles en KiB décrivent l’encombrement compilé et ne prédisent pas la durée d’un callback Windows. Une polyphonie matérielle annoncée ne prouve pas la capacité d’empiler six instances dans Modular.
- [Répertoire communautaire](https://github.com/aroum/fm1-custom-fw) : point d’entrée vers les firmwares et leurs dépôts, utile pour poursuivre l’examen des moteurs.

## Pistes concrètes

| Piste | Dépôt d’origine | Intérêt et travail à prévoir |
| --- | --- | --- |
| FM six opérateurs | [MSFA](https://github.com/google/music-synthesizer-for-android), Apache‑2.0 ; [Melodee](https://github.com/keremimo/melodee) via fm1-dock | Algorithmes, enveloppes, compatibilité DX7. Vérifier les fichiers exacts et les licences de chaque adaptation. À placer comme source de chaîne, sans empiler automatiquement une nouvelle voix sur le signal. |
| Modèles physiques | [DaisySP](https://github.com/daisyaudio/DaisySP) et composants Plaits indiqués dans les notices officielles | Cordes pincées, percussions ; bon complément aux excitations. Licence à vérifier composant par composant et coût à mesurer par voix. |
| Synthèse analogique | [Open303](https://github.com/RobinSchmidt/Open303), MIT d’après les notices officielles | Source acid et filtre ; séparer le générateur de la partie traitement pour respecter la chaîne. |
| Macro-oscillateur | [GATOR](https://github.com/npi3pak/fm1-gator), cité par fm1-dock | Beaucoup de modèles ; un portage implique dépendances, initialisation, taux d’échantillonnage et budget CPU. |
| Kalimba modale | [FiMba](https://github.com/jadamsowers/fm1-fimba), cité par fm1-dock | Excitations et corps résonants ; comparer aux corps déjà présents avant de dupliquer le coût. |

Aucun moteur tiers n’a été copié ou lié dans cette alpha. Les nouveaux Ring mod, Bitcrusher, Phaser et Comb sont des traitements écrits pour Modular. L’étape suivante d’un portage devra épingler un commit, lire les en-têtes de chaque fichier retenu, conserver les notices applicables et mesurer la synthèse indépendamment puis dans une chaîne de six traitements. L’étude ne promet ni identité sonore avec un FM‑1 ni compatibilité de presets DX7 dans la FM actuelle à deux opérateurs.
