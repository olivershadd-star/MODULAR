# Modular 1.4 alpha 2 — fenêtre et accès aux commandes

Le minimum précédent de 1000×760 pouvait dépasser la zone de travail d’un portable, particulièrement avec un facteur DPI Windows élevé. La fenêtre autonome JUCE ne proposait pas de bouton d’agrandissement.

L’éditeur utilise maintenant une taille initiale plafonnée à 960×640, réduite selon la zone de travail logique de l’écran, avec des marges pour la barre de titre et les bordures. Le minimum descend à 480×360, ou moins si l’écran l’impose. Le clavier reste hors de la zone défilante et ne bouge pas avec elle.

La zone de travail adapte les outils de source/presets, dispose les modules sur une ou deux colonnes et défile verticalement lorsque nécessaire. Les potards ne sont pas écrasés pour faire tenir six modules dans une hauteur insuffisante. Le séquenceur se trouve à la fin de cette zone.

Dans l’application autonome seulement, les trois boutons réduire, agrandir/restaurer et fermer sont activés sur la fenêtre JUCE. Sur Windows, le bouton d’agrandissement standard utilise SW_SHOWMAXIMIZED, sans mode kiosque qui masquerait la barre. La taille et la position normales sont recadrées dans la zone de travail après déplacement ou changement de moniteur ; le recadrage attend la fin du glissement de souris et ne modifie pas une fenêtre agrandie ou réduite. Le VST3 ne commande pas les boutons de son hôte.

Les tests vérifient plusieurs zones de travail dont des coordonnées négatives de moniteur secondaire, le clavier et la zone défilante dans cinq dimensions de fenêtre, l’accès au séquenceur après défilement et la présence des trois boutons avec un cycle agrandir/restaurer sur une fenêtre de test. Des captures Windows sont générées à 960×600 et 760×480. Les essais réels de changement d’écran et de facteur DPI sur le PC utilisateur restent à confirmer.

Aucun changement de moteur DSP ni de paramètres audio dans cette révision.
