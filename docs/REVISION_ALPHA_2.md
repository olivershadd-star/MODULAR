# Révision 1.2 alpha 2

L’interface classique et la case MOTEUR RACK sont retirées. La grille du rack reste toujours visible. Le moteur classique et ses identifiants de paramètres sont conservés pour la lecture des anciens projets. Un bouton de migration est proposé uniquement pour ces anciens états.

Optimisation du DSP : l’historique partagé par les grains, delay et chorus utilise un index circulaire incrémental et des corrections bornées plutôt que des divisions entières par lecture. La taille des buffers et l’interpolation restent identiques. L’EQ ne recalcule ses coefficients que dans le mode EQ et lors d’un changement effectif. L’accord des corps est ignoré pour les modules qui ne sont pas des résonateurs.

Validation locale : suites core, sequencer, rack et effects. Le test rack vérifie aussi les interpolations après plusieurs tours d’historique et après remise à zéro. Un banc de charge résonateur + deux granulaires (densité 100, grains 500 ms, transposition +12, 1/8/16 voix, mouvement de brillance) conserve les mêmes sommes de sortie avant/après. Les temps CPU de la machine partagée varient fortement et ne permettent pas de garantir une performance sur le PC utilisateur.

La compilation Windows et les tests JUCE sont effectués par GitHub Actions. La disparition des craquements doit encore être vérifiée dans l’hôte avec le même preset, le même nombre de voix et le même buffer audio.
