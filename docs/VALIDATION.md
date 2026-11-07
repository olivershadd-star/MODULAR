# Validation — V1.2 alpha 1

## Scénario de référence

Retour utilisateur : VST3 dans Ableton, WAV importé, notes d'un clip MIDI, seul bloc A utilisé. Les effets doivent répondre avec leurs réglages usuels; il ne faut pas maintenir artificiellement une note ou boucler la source pour les réveiller.

## Causes corrigées

En V1.1, l'enveloppe post-chaîne supprimait les sorties retardées et détruisait la voix. Le granulaire pouvait chercher dans un historique négatif/non enregistré. Le chorus mono à une ligne ne donnait pas l'effet multivoix attendu. En V1.2, l'enveloppe du rack est appliquée à l'excitation, les queues continuent après note-off, les grains capturent un fragment valide et les canaux stéréo sont conservés.

## Tests exécutés en natif Linux

Sources officielles JUCE 8.0.6, GCC, six séries CTest :

- **core** : moteur classique, 224 combinaisons phase/décroissance, niveau unitaire, gains, A/B, routages, sustain, canaux, vol de voix et charge.
- **sequencer** : horloges, positions, blocs de tailles différentes, gate, silences, arrêts et indépendance du clavier.
- **rack** : types de modules, transparence du vide, instances indépendantes, ordre audible, EQ plat/actif, réception réelle des blocs précédents par le granulaire, LFO, stéréo/états, changement de topologie et charge 16 voix.
- **effects** : note MIDI de 75 ms avec relâchement de 10 ms et delay full wet; premier écho à 250 ms, second à 500 ms avec décroissance, déplacement à 500 ms quand le temps change, absence de deuxième écho sans feedback. Note de 20 ms avec granulaire full wet : démarrage audible avant 20 ms, grains conservés après note-off et réponses distinctes de taille/densité/hauteur. Pas de chemin direct ajouté. Chorus stéréo, modifications de profondeur et vitesse, maintien de la stéréo dans un filtre placé après lui, invariance de taille de callback et trois modes de distorsion distincts.
- **plugin** : processeur JUCE réel avec source embarquée et événements MIDI de type clip hôte. Contrôles d'énergie du granulaire avant note-off, écho après note-off et canaux distincts du chorus, uniquement dans le bloc A. Décodage des douze WAV embarqués, restauration portable de leur état, préservation source/recettes, lecture d'une ancienne recette V1.1, identité des instances après déplacement, index des anciens paramètres inchangés et correspondance du knob réel avec l'index publié au wrapper pour Configure. Gestes liés et indépendants des corps depuis l'éditeur.
- **audio_io** : import et rejet des fichiers invalides.

Le test effects passe aussi sous AddressSanitizer et UndefinedBehaviorSanitizer, détection des fuites désactivée dans cet environnement. Le moteur de test peut produire des mesures numériques et vérifier les événements; ces résultats ne constituent pas une écoute humaine dans Ableton.

## Interface et formats

Vue principale inspectée par rendu natif à 1060×700 et 900×620 : source, grille 2×2, knobs, séquenceur et clavier visibles ensemble. Les détails secondaires sont dans un callout rattaché à l'éditeur. Les captures ne constituent pas un test de mise à l'échelle Windows réelle.

VST3 et Standalone sont compilés en natif Linux. Les ressources audio sont liées aux deux exécutables; le chargement interne utilise leurs octets compilés, pas des chemins de fichiers. La livraison Windows ne copie plus de dossier Excitations séparé.

## Vérification Windows restant à faire

Le workflow fourni construit les deux formats sous Windows et relance ces six séries. Il n'a pas été exécuté ici. L'installation, le scan, l'écoute, Configure/macros/MIDI Map dans Ableton, la manipulation réelle des menus/callouts et du glisser-déposer, le redimensionnement à 300 %, la charge sur le PC utilisateur et le chargement dans Curve Pilot restent à valider dans ces applications.

Les 16 voix et la limite de queue de 30 s peuvent couper une queue lors d'un vol de voix ou à la limite maximale. Les retards créatifs ne sont pas compensés comme une latence hôte. Les limites DSP, formats et sauvegardes sont décrits dans README.md.
