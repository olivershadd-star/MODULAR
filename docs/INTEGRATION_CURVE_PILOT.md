# Vérification dans Curve Pilot

Le travail précédent a inspecté l'archive Curve Pilot V2 : transmission du playhead aux plugins par blocs de 32 échantillons, paramètres par identifiants stables et sauvegarde de l'état. Cette livraison n'a pas audité la version V2.1 plus récente ni exécuté le rack Windows.

Les tests du processeur Resonant Lab simulent le playhead et les blocs de 32 échantillons. Le résultat dans la véritable application doit encore être vérifié.

1. Charger le nouveau VST3 dans un emplacement instrument, sans ancienne version Arlc en doublon.
2. Dans un hôte avec transport, choisir Hote pour le séquenceur; vérifier départ, arrêt et changement de BPM. Dans une application autonome sans playhead valide, choisir Interne et PLAY.
3. Mapper m1_cutoff ou m1_b1_brightness, puis échanger les emplacements. Vérifier que le mouvement suit toujours l'instance 1.
4. Utiliser deux instances du même type, avec des réglages distincts. Placer éventuellement Triple Disto après Resonant Lab, à niveau modéré.
5. Sauver le projet Curve, fermer, déplacer le WAV original puis rouvrir. Vérifier source, types, ordre, séquence et automations.

Les réglages classiques a_*/b_*, cutoff/drive/mix/balance/route sont encore publiés pour les anciens projets; ils commandent le moteur classique quand rack_on est coupé. Utiliser les mN_* pour le rack moderne.

Si le chemin ou le nom du binaire a changé, sélectionner de nouveau le VST3 dans le rack. Ne pas écraser l'unique copie d'un ancien projet avant ce contrôle.
