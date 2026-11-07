# Évolution V0.2 — 2 novembre 2026

## Objectif atteint dans le code

Une excitation audio peut être jouée chromatiquement et alimenter deux résonateurs d’une couche. Le couple source/corps devient accessible grâce à des réglages séparés. La chaîne est désormais :

Source interne/importée → transposition et vélocité → MAIN/SUB → mélange source/corps → enveloppe → gain lissé → saturation douce de sortie.

Ce schéma est celui du prototype ANKS. Il ne prétend pas reconstituer le chemin DSP original du WSA1. Le fichier HISTORIQUE_RECHERCHE_V0_1.md conserve l’étude précédente; ses mentions de fonctionnalités absentes concernent la V0.1. Le README et le présent document font référence pour la V0.2.

## Accord

Le filtre à un pôle et l’interpolation dans la boucle produisent une phase supplémentaire. La longueur du retard est maintenant calculée pour que leur somme atteigne 2π à la fondamentale, ou π avec un feedback inversé. Les essais portent sur 224 combinaisons de notes, débits, brillance et polarité. Ils vérifient cette condition de phase et la décroissance, pas une identité timbrale avec le WSA1.

Les pertes de l’interpolation et du filtre ne sont pas compensées en gain afin de garder une boucle passive. Conséquence : une brillance faible raccourcit certains partiels et la durée affichée est nominale. La stabilité de ce moteur ne vaut pas pour les futurs couplages.

## Chargement et sauvegarde

Le décodage s’effectue hors de la fonction de rendu audio. Les données sont remplacées sous verrou, après arrêt des voix. Le rendu tente d’acquérir le verrou sans attendre; en cas de concurrence, un bloc silencieux et un arrêt des voix au bloc suivant empêchent les notes bloquées.

La sauvegarde incorpore les octets du fichier original et son nom. Le décodage et les fondus de bord sont reproduits à la restauration. Le test de sérialisation porte sur le contenu binaire; la restauration d’un projet Ableton reste à valider.

## Interface

Dix commandes rotatives, trois commutateurs et un clavier. Fenêtre initiale 1100 × 780; dimensions ajustables entre 760 × 620 et 1600 × 1100. Le code UI est vérifié syntaxiquement, sans validation visuelle dans un hôte. La lisibilité à 300 % sous Windows reste à tester sur l’écran de l’utilisateur.

## Étape suivante

Compiler et charger cette couche dans Live; écouter les excitations seules puis leur transformation. Vérifier import, pédale, pitch-bend, réouverture du projet et limites de niveau. Stabiliser ce comportement avant d’ajouter trois autres couches et leur interaction.
