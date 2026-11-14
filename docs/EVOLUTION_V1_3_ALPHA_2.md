# 1.3 alpha 2 — source FM

FM à deux opérateurs sinusoïdaux placée avant la chaîne. Rapport 0,125–16, index 0–12, feedback 0–0,95, sensibilité à la vélocité, ADSR de modulation indépendante et enveloppe de volume. La source remplace le WAV, sans superposition. Les traitements et leur ordre restent indépendants.

Les onze paramètres fm_* sont ajoutés après tous les paramètres 1.3 alpha 1. Identifiants, indices et listes de choix existants inchangés. Les créations complètes sauvegardent la FM ; les chaînes/modules préservent la source courante. Les états sans paramètres FM fournissent les valeurs par défaut et FM désactivée. Le chargement WAV/factory sélectionne ce WAV.

DSP original dans FmSource.h, modulation de phase et feedback retardé d’un échantillon interne, 2× fixe et FIR de décimation 63 points. Amplitude et mémoires bornées, contrôles lissés, pas d’allocation audio. Retard de source environ 15 échantillons ; pas de compensation hôte ajoutée. Les notes/rapports extrêmes et le feedback peuvent encore produire de l’aliasing ; le modulateur est borné en fréquence.

Tests FM : porteuse seule à index zéro, bandes latérales, action du rapport/feedback/vélocité, décroissance de modulation, relâchement, remise à zéro, valeurs extrêmes de fréquence d’échantillonnage, invariance des callbacks, chaîne granulaire/résonateur/semi-modulaire, ordre et absence de couche WAV. Tests JUCE : rendu MIDI, état complet, source conservée par recette de chaîne, migration sans FM et sélection de source par chargement factory.

Banc de charge local à 48 kHz : source FM seule sur 1/8/16 voix, mesure CPU indicative sur machine partagée. Vérification supplémentaire du décimateur contre une référence idéale synthétisée à 16×, pour une porteuse 4 kHz / rapport 2 / index 4. Les résultats numériques ne remplacent pas l’écoute dans l’hôte.
