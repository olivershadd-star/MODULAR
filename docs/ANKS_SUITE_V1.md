# Suite ANKS — contrat Resonant Lab 1.2 alpha

Curve Pilot : rack hôte VST3. Resonant Lab : instrument VST3 ou autonome. Triple Disto : effet externe. Les modules internes de Resonant Lab ne nécessitent pas l'hébergement d'effets VST externes.

L'interopérabilité repose sur VST3/JUCE : audio stéréo (source mono et traitement stéréo du rack), entrée MIDI, paramètres normalisés, playhead BPM/PPQ/lecture et sauvegarde complète de l'état. Aucun protocole privé requis.

Identité conservée : fabricant Anks, code Arlc, produit ANKS Resonant Lab, racine d'état ANKS3. Métadonnées anksSuiteSchema=3, instrumentVersion=1.2.0-alpha.1. Anciennes sauvegardes sans rack : restauration sur le moteur classique. Sauvegarder ses projets avant remplacement du binaire et éviter deux versions Arlc simultanément scannées.

Tous les anciens IDs restent publiés et dans le même ordre. Les nouveaux paramètres sont ajoutés : rack_on (choix du moteur), rack_order (une des 24 permutations), waveform, puis m1_ à m4_ avec leurs types et réglages. Le numéro mN est l'identité de l'instance, jamais son emplacement visuel. Un déplacement modifie uniquement rack_order; une automation m1_cutoff suit l'instance 1. Remplacer son type ne redirige pas cette automation : elle reste stockée pour le filtre de l'instance.

Le lien des gestes des deux corps est une fonction d'interface. Les deux paramètres d'automation restent indépendants. La chaîne et ses modulations internes sont sauvegardées dans .ankschain; le WAV, les réglages de source et le séquenceur restent hors de cette recette. Les créations .anksres et l'état hôte embarquent le WAV et tous les paramètres.

Séquenceur : Hote suit une position BPM/PPQ valide et isPlaying. Interne suit son BPM/PLAY, utile dans un rack autonome sans transport. Le LFO musical du filtre suit le PPQ de l'hôte, se fige avec son arrêt et utilise une phase interne quand aucun playhead valide n'est disponible. Le LFO en Hz reste libre. Aucun MIDI séquencé n'est envoyé à l'extérieur.

Tous les nouveaux blocs travaillent par voix. Aucun module global après sommation dans cette version. Les retards créatifs du granulaire/delay ne sont pas déclarés comme une latence de traitement à compenser. Les queues du rack survivent au relâchement de la source, dans un budget borné de 30 s; le vol de voix et les commandes panic peuvent les couper.

La palette suit les conventions précédentes : fond #101723, cartes #1c2738, texte #edf2ff, secondaire #91a2be, accent #7ce2ed, second accent #ffae70. La fenêtre reste défilante et redimensionnable.

Le contrat et les tests de processeur ne constituent pas un essai du rack réel sous Windows. Aucun code Curve Pilot ou Triple Disto n'est modifié ni compilé ici.
