# Correctif V1.2 alpha 1

Déclencheur : essais utilisateur sous Ableton VST3, WAV importé, notes jouées par clip MIDI, un seul bloc A. Le granulaire et le delay étaient silencieux; le chorus trop faible. L'interface en barrettes et cadres empilés imposait un défilement excessif.

- Enveloppe de source avant les modules du rack et gestion bornée des queues après note-off. Moteur classique conservé.
- Granulaire refait : fragments réellement capturés, démarrage après environ 5 ms, recul limité à l'historique valide et lecture bouclée des fragments sous fenêtre, sans injection de son direct à 100 % wet.
- Delay vérifié après gate MIDI de 75 ms; répétitions chronométrées et décroissance mesurée.
- Chorus à trois voix par canal avec phases distinctes, préservation de la stéréo dans les blocs suivants.
- Disto douce / dure / wavefolding; quatre paramètres de mode ajoutés après les anciens paramètres.
- Interface à knobs, quatre cadres 2×2, cinq commandes partagées des corps + balance et mix, séquenceur sur la même vue. Détails secondaires en callout.
- Index de paramètre retourné au wrapper pour les contrôles; gestes d'automation explicites et identités fixes après déplacement.
- Douze excitations compilées en ressources VST3/Standalone. Import WAV séparé des chargements de presets pour éviter les dossiers apparemment vides.
- Compatibilité de lecture des états et recettes V1.1 avec ajout du mode de distorsion doux.

Les preuves de test et les limites de validation réelle figurent dans VALIDATION.md.
