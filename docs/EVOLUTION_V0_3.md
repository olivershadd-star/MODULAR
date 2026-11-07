# ANKS Resonant Lab — réalisation V0.3

Date : 2 novembre 2026. Auteur : ANKS.

## Périmètre de cette réalisation

Le prototype à une couche devient un sampler qui alimente un double corps résonant. A et B ont leurs propres paramètres. La même rangée de commandes édite A, B ou les deux selon le sélecteur. Lier n'écrase pas les différences déjà établies : seul le prochain paramètre touché est envoyé aux deux.

Un filtre passe-bas, une saturation et le bloc double corps peuvent être activés ou contournés et permutés. Les six chemins sont traités par le DSP, pas seulement représentés graphiquement. Le filtre placé avant le corps modifie son excitation; placé après, il modifie sa sortie. La brillance interne reste un paramètre de perte dans chaque corps.

Le bloc A/B forme une paire en parallèle, sans couplage entre eux. Le mélange est pondéré pour que deux corps identiques n'ajoutent pas artificiellement un doublement de niveau. Cela n'empêche pas les renforcements ou annulations propres aux réglages différents.

## Gain

Le coefficient fixe 0,25 de la sortie V0.2 disparaît. Les gains d'entrée et de sortie utilisent des dB, avec plage −24/+24 et zéro par défaut. Les niveaux A/B ont leur propre plage −24/+12 dB.

Le signal direct conserve son amplitude à vélocité maximale, avec attaque zéro. L'import ne transforme plus les premiers échantillons par un fondu. Les sources stéréo restent réduites en mono; cette opération peut changer le niveau selon leur corrélation.

Le garde-fou de sortie est un écrêtage dur à ±0,98. Le niveau affiché est la crête avant cet écrêtage. Ce choix rend les dépassements visibles; il ne remplace pas un limiteur audio transparent.

## Contrôle pendant le jeu

Les paramètres sont pris en compte au début de chaque bloc audio, puis les gains, mélanges, pertes et positions continus sont lissés. Les réglages d'accord déplacent progressivement les retards, ce qui peut créer un glissement de hauteur. Le type harmonique/impair est un changement discret.

Le déplacement d'un module applique une descente/remontée d'environ 4 + 4 ms autour du changement d'ordre. Les notes continuent, sans remise à zéro de la lecture. Les contrôleurs MIDI et l'enveloppe continuent de fonctionner pendant ces modifications.

Les paramètres A et B d'automation restent séparés dans l'hôte. Le mode lié est une commodité d'édition de l'interface, pas une contrainte imposée aux automations enregistrées.

## État et compatibilité

Nouvel identifiant de plugin Arlc et état ANKS3, pour installer la V0.3 à côté de la V0.2. La restauration conserve les paramètres, l'ordre, le mode d'édition, le nom de l'excitation et les octets audio incorporés. La lecture est réinitialisée à la réouverture; les voix en cours ne sont pas sérialisées.

Le chemin principal de construction redevient JUCE 8.0.6 avec Visual Studio dans GitHub Actions. Le workflow, sa copie visible et le script portent toujours les mêmes noms. Les sources du framework sont récupérées automatiquement à un commit fixé.

## Banque de départ

Douze WAV synthétiques originaux sont fournis. Les huit ajouts sont reproductibles avec tools/generate_extra_excitations.py, qui utilise Python et NumPy. La construction du VST n'a pas besoin d'exécuter ce script : les WAV sont déjà inclus.

## Étapes ultérieures possibles

Points de boucle et découpe du WAV, lecture stéréo, rééchantillonnage amélioré, autres familles de corps, couplages contrôlés, enveloppes et courbes de mouvement. Ces fonctions ne font pas partie de la V0.3 livrée.
