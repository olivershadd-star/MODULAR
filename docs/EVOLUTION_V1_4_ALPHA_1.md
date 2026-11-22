# Modular 1.4 alpha 1

Nom du produit, VST3, exécutable et livraison Windows : Modular. Les codes d’identité VST3 restent inchangés ; le générateur d’identifiants JUCE 8 avec JUCE_VST3_CAN_REPLACE_VST2=0 utilise ces codes, indépendamment du nom affiché.

Six emplacements, 720 permutations, les 24 permutations initiales conservées aux mêmes indices. E/F sont ajoutés après les quatre instances dans les anciens états. Les paramètres des deux nouveaux emplacements et les quatre nouveaux paramètres FX sont ajoutés après tous les anciens paramètres. Chaînes/module : schéma 4 ; migration de schémas 1–3. Les automations normalisées des listes étendues demandent une vérification dans l’hôte.

Palette graphite/rouge, blanc cassé et jaune. Logo vectoriel, traits gravés statiques, potards fins, trois rangées de deux cadres et fenêtre redimensionnable. Aucun dessin dans le thread audio ; pas d’animation de fond.

Quatre traitements ajoutés : ring mod, bitcrusher, phaser six étages et peigne accordé. Oscillateur de modulation par récurrence, coefficients mis en cache ; mémoire de peigne dans le stockage déjà prévu. Chaque traitement respecte OFF, le signal entrant et les états stéréo indépendants. Le bitcrusher produit volontairement de la distorsion et de l’aliasing. Le phaser combine les chemins direct et all-pass dans son traitement ; le peigne peut produire un silence initial égal à son retard à 100 % wet. Aucun nouveau générateur ne se superpose automatiquement à la source.

Les correctifs alpha 4 du bypass et des grains restent actifs. Le nombre d’emplacements et la mémoire allouée augmentent : mesurer sur le PC cible. Les bancs ajoutent FM + six traitements mixtes et six résonateurs, à 44,1/48/96 kHz et buffers 64/128/256, avec 1/8/16 notes. Les mesures GitHub sont celles du runner, sans promesse de performance identique sur une autre machine.

Les tests ajoutent les 720 permutations, la conservation des anciens ordres, les nouveaux traitements à trois fréquences d’échantillonnage, la migration de chaînes antérieures et le déplacement A/F avec leurs paramètres. Des captures de l’interface réelle sont générées sous Windows aux dimensions par défaut et minimales.
