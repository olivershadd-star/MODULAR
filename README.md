# ANKS Resonant Lab 1.3 alpha 1

Instrument C++ / JUCE 8.0.6 — VST3 et application autonome Windows x64. Auteur : ANKS.

Cette mouture conserve la banque de 36 excitations et ajoute trois traitements réordonnables : **Semi-modulaire**, **Décalage de fréquence** et **Freeze spectral**. La source reste unique, en amont du rack : WAV/factory ou onde interne (impact, sinus, triangle, dent de scie, carré). Aucun de ces traitements n’ajoute automatiquement un oscillateur.

### Semi-modulaire

Inspiré de fonctions de synthèse semi-modulaire de type Mother-32, il traite le signal entrant. VCF 24 dB/octave passe-bas ou passe-haut, résonance, drive, VCA, ADSR et LFO sinus/triangle. Ce n’est pas une émulation du circuit Moog. Deux patches dans `…` permettent de choisir **Aucune / LFO / Enveloppe / Suivi entrée** vers **VCF / VCA**, avec dosage bipolaire. Par défaut : enveloppe → VCF, LFO → VCF. Le VCA suit l’enveloppe MIDI ; « VCA ouvert » laisse passer le son sans ce contrôle d’amplitude. Attaque, déclin, filtre, résonance, LFO et dosage du patch 1 sont en vue principale ; maintien, relâchement, drive et routage complet sont dans `…`. Les patches VCF couvrent ±5 octaves ; les patches VCA ajoutent ou retirent de l’amplitude bornée entre 0 et 1.

### Décalage de fréquence

Déplacement signé en Hz, indépendant d’une transposition musicale. Produit battements et timbres inharmoniques. Réglage ±2000 Hz, Mix et Sortie. Traitement stéréo indépendant, avec un filtre de Hilbert FIR de 127 points ; séparation des bandes moins précise tout en bas du spectre. Retard interne de 63 échantillons, compensé entre les branches dry/wet du module.

### Freeze spectral

Le bouton **Maintenir** capture le spectre courant après la mise en route de l’analyse, puis le prolonge même si l’entrée se tait. Décocher retourne à l’analyse en direct ; décocher puis recocher capture un autre instant. FFT 1024 points, pas de 256, recouvrement de Hann et progression de phase. Retard interne 1024 échantillons (21,3 ms à 48 kHz), dry/wet alignés. Capture et relâchement évoluent sur les fenêtres d’analyse. Une capture du silence reste silencieuse. Le maintien respecte la limite de queue de 30 secondes après extinction de l’enveloppe source et peut être coupé par un vol de voix.

Les trois traitements fonctionnent à n’importe quelle position et conservent la stéréo. Les effets à retard sont des transformations créatives dans une architecture par voix : la latence globale hôte reste déclarée nulle, sans compensation automatique du trajet complet. Éviter de les comparer à une chaîne parallèle sans alignement.

## Banque d’excitations

36 sources embarquées, classées en cinq familles : Classiques (les 12 originales), Bois et cordes, Métaux, Textures organiques et Matières numériques (6 nouveautés par famille). Les anciennes sources et leurs indices restent identiques. Les 24 ajouts sont des synthèses originales, mono PCM16 / 48 kHz, crête proche de −6 dBFS, durée 1,6 à 2,4 s. Voir `Excitations/BANQUE_FACTORY.md`.

## Prise en main

1. Copier le dossier complet ANKS Resonant Lab.vst3 dans le dossier VST3 Windows, puis scanner dans Ableton.
2. Choisir une **Excitation intégrée** ou cliquer directement **Importer WAV**. Les 36 excitations sont compilées dans le VST3 et dans l'application : aucun dossier de WAV à installer à côté.
3. Jouer un clip MIDI dans Ableton, choisir le module de A et cocher ON. B/C/D peuvent rester vides.
4. Le knob Mix indique le pourcentage traité. À 100 %, le granulaire, le delay et le chorus donnent uniquement le signal traité : aucun son direct de secours n'est ajouté.

Le granulaire produit ses premiers grains après environ 5 ms de capture. Le delay donne son premier écho au temps choisi (250 ms par défaut), même si la note MIDI est terminée avant. Le chorus utilise trois voix modulées par canal et produit une sortie stéréo.

## Interface et contrôle Ableton

En haut : import, excitations intégrées, presets, choix de source et réglages globaux. Au centre : quatre emplacements en grille 2×2, lus **A → B → C → D**. En bas : séquenceur et clavier. La vue du rack n'utilise plus de défilement. Fenêtre par défaut 1060×700, minimum 900×620; contrôler le résultat réel à 300 % Windows.

Glisser l'en-tête `::` d'un cadre sur un autre échange les instances avec leurs réglages et mémoires. Les numéros 1–4 indiquent leur identité; les lettres A–D indiquent leur position. Le bouton `…` ouvre les commandes secondaires du module dans un volet défilant.

Le double corps partage les cinq commandes Résonance, Brillance, Position, Accord et Niveau entre **Corps 1 / Corps 2 / 1+2 liés**. Passer en lié ne copie pas les valeurs : le prochain geste applique la même valeur aux deux corps. Leurs automations restent distinctes. La balance donne la part du corps 2 (0 % = corps 1 seul, 100 % = corps 2 seul).

Dans Ableton : **Configure → manipuler les knobs → assigner aux macros du rack → MIDI Map du contrôleur**. Les commandes principales et secondaires adressent les vrais paramètres du processeur; l'éditeur fournit aussi leur index au wrapper VST3. Les anciens identifiants et leur ordre sont conservés; les nouveaux contrôles sont ajoutés après tous les paramètres 1.2, modes de distorsion compris. Les identifiants et les indices des anciens contrôles sont conservés. Les listes de types et d’ondes sont étendues : vérifier les automations hôte enregistrées en valeurs normalisées lors d’un changement de version.

Une automation `m1_…` suit toujours l'instance 1 après déplacement. Changer de type ne réaffecte pas cette automation à une autre fonction; les réglages inactifs restent mémorisés. Les réglages communs Mix/Level et la couleur partagée saturation/delay restent communs à l'instance. Pas d'undo dans cette alpha.

## Modules

| Module | Principales commandes |
| --- | --- |
| Double corps | Résonance, brillance, position, accord, niveau par corps; balance, lien et mélange. Activations indépendantes; variante impaire dans `…`. |
| Filtre / EQ | Passe-bas, passe-haut, passe-bande, coupe-bande; fréquence, Q et mélange. LFO Hz ou synchronisé, profondeur en octaves, sinus/triangle. EQ quatre bandes avec gains en vue principale, fréquences et Q dans `…`. |
| Disto / Saturation | Douce tanh, écrêtage dur ou wavefolding; Drive, Tone, Mix, Level. |
| Granulaire | Fragments du signal entrant : taille, densité, recul, dispersion, hauteur et mélange. Jusqu'à 16 grains par canal et par voix. |
| Delay | Temps en secondes, feedback, couleur, mélange et niveau. Répétitions conservées après note-off. |
| Chorus | Trois voix retardées et modulées par canal, phases gauche/droite distinctes; vitesse, profondeur, mélange et niveau. |
| Décalage de fréquence | Déplacement signé en Hz, mélange et niveau. |
| Freeze spectral | Capture / maintien du spectre, mélange et niveau. |
| Semi-modulaire | VCF, VCA, ADSR, LFO et deux routes de modulation. |

Le LFO concerne le filtre à variable d'état, pas les bandes de l'EQ. En mode EQ, les commandes LFO sont masquées. Les boutons Sync/Triangle sont directement accessibles dans le filtre. Une vitesse synchronisée s'exprime en temps et suit BPM/PPQ; le mode Hz reste libre.

Les modules travaillent par voix et préservent désormais les canaux stéréo des blocs précédents. Le double corps et la saturation gardent leur principe DSP. La source WAV reste réduite en mono à l'import; le chorus peut ensuite créer de la stéréo.

## Ce qui était défectueux en V1.1

L'enveloppe était appliquée après toute la chaîne : elle étouffait les échos et grains avant qu'ils aient pu être entendus, puis supprimait la voix. Le granulaire pouvait lire un historique négatif ou non encore rempli. Le chorus ne possédait qu'une ligne mono, sans épaississement multivoix.

Dans le rack 1.2, l'enveloppe contrôle l'excitation **avant** les modules. La voix reste disponible pour leurs queues après le relâchement. Le granulaire capture des fragments effectivement enregistrés et adapte le recul à l'historique réellement disponible au démarrage. Il peut répéter ces fragments avec une fenêtre à attaque courte; le réglage de hauteur modifie leur vitesse de lecture, sans lire le futur. Le chorus a des états gauche/droite indépendants.

Le moteur classique est conservé pour les anciens projets V0.3/V1 sans rack. Les anciens presets V1.1 de rack sont lisibles, mais leurs relâchements et effets changent volontairement avec la correction; aucune identité sonore avec les bugs précédents n'est promise.

## Sources et presets

- **Importer WAV** accepte WAV/AIFF jusqu'à 10 s / 16 Mo, 8–192 kHz. Il conserve la chaîne et le pattern. La boucle reste celle du fichier entier.
- **Excitations intégrées** charge une des 36 sources ANKS et sa racine suggérée. Aucun fichier audio externe n'est lu pour cette opération.
- WAV décoché : Impact interne, Sinus, Triangle ou Dent de scie.
- **Création complète / .anksres** : source embarquée, tous les réglages et séquence. Le projet hôte sauvegarde le même état complet.
- **Chaîne / .ankschain** : instances, ordre, modulations, entrée/sortie; conserve la source, la racine, la boucle, l'enveloppe et le pattern courants.
- **Module A/B/C/D / .anksmod** : type et réglages chargés dans l'instance qui occupe la destination sélectionnée.

Les recettes V1.1 sont migrées avec le mode de distorsion doux par défaut. Les nouveaux fichiers de recette portent le schéma 3 (lecture des schémas 1/2 avec valeurs par défaut pour les nouveaux contrôles); l'état complet garde la racine ANKS3 et le schéma de suite 3. Le chargement d'un son ou d'un preset coupe les voix courantes. Un séquenceur interne actif redémarre au premier pas. Ajouter l'extension demandée lors de la sauvegarde.

Le dossier Excitations reste dans les **sources de compilation** pour construire les ressources; il n'est plus copié dans l'archive Windows à installer.

## Séquenceur

Seize boutons numérotés choisissent le pas édité; les cases dessous activent ses notes. Les knobs Note/Vélocité/Durée modifient le pas sélectionné. Longueur 1–16, résolutions 1/4 à 1/32, canal 1–16.

Hote suit le transport d'Ableton. Interne utilise PLAY et le tempo réglé dans l'instrument, notamment en autonome. Les notes du séquenceur ne sont pas envoyées à d'autres plugins. Le clavier et les clips MIDI restent utilisables en parallèle.

## Compilation et installation

Même procédure : ZIP de sources → extraire → déposer le contenu dans GitHub → **Actions → VST3 Windows**. Télécharger l'artifact **ANKS-Resonant-Lab-V1-3-Alpha-Windows**, puis extraire son ZIP Windows. Il contient le VST3 et Standalone, avec les WAV compilés dedans.

Si le workflow manque, créer `.github/workflows/build-vst3.yml` et copier le fichier visible `build-vst3.yml`. Le script utilise le commit officiel JUCE 8.0.6 fixé, construit les deux formats et lance huit séries de tests, dont le contrôle de la banque si Python est disponible. En local : Visual Studio C++, SDK Windows et CMake, puis Construire_Windows.ps1.

Identité VST3 conservée `Anks/Arlc`. Ne pas scanner simultanément plusieurs versions de même identifiant; conserver les anciennes archives et une copie des projets. Un ancien chemin de plugin peut demander une nouvelle sélection dans Curve Pilot.

## Limites

16 voix; les queues peuvent être coupées lors d'un vol de voix, d'un panic, d'un changement de source ou de type. Budget de queue borné à 30 s après l'enveloppe, avec fondu final; les très longs feedbacks ne sont pas infinis. Un changement d'ordre utilise une atténuation de 4 ms puis une remontée de 4 ms. Le nombre de voix, l'historique stéréo et la fréquence d'échantillonnage influencent CPU/RAM; environ 100 Mo de stockage DSP à 48 kHz, jusqu’à 265 Mo à 192 kHz, hors sources/framework/hôte. Les buffers sont alloués à la préparation, pas par échantillon.

Le granulaire live est causal : son recul se limite à ce qui a effectivement été capturé. Il n'a pas encore de gel ni de stretch indépendant. Le delay n'est pas encore synchronisé au tempo. Pas d'export WAV, de découpe début/fin, ni de catalogue avec préécoute dans cette version. L'oscillation simple, la transposition interpolée et la distorsion sans suréchantillonnage peuvent produire de l'aliasing.

Protection finale par écrêtage à ±0,98 et bornage intermédiaire ±16; ce n'est pas un limiteur transparent. Pas d'émulation exacte du Technics WSA1. Les tests natifs ne remplacent pas l'écoute et les essais Windows/Ableton/Curve Pilot. Voir docs/VALIDATION.md.
