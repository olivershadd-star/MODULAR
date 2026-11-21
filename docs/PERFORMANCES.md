# 1.3 alpha 3 — charge audio et préservation du rendu

Cette étape garde quatre emplacements et toutes les possibilités sonores. Elle ne réduit ni le nombre maximal de voix, ni les grains, ni les contrôles, ni la fréquence des modulations, ni les queues d'effets. Les huit emplacements attendent la validation de cette base.

## Travail effectué

- Les mémoires des résonateurs sont invalidées par un compteur de validité au lieu d'être effacées intégralement à chaque nouvelle note. Les zones non écrites sont lues comme zéro, y compris aux frontières de l'interpolation circulaire.
- Seuls les résonateurs utilisés sont réaccordés. Les deux canaux partagent leurs coefficients d'accord, mais gardent des mémoires audio et états de lissage distincts. L'ancien moteur est initialisé à la demande en préservant ses conditions initiales pour les anciens projets.
- Les gains, enveloppes et coefficients inchangés sont conservés. Le budget de queue est calculé une fois dans l'instantané commun, plutôt qu'une fois par voix. Les constantes de fenêtre des grains sont préparées à leur naissance, sans changer les divisions ni l'interpolation.
- Un freeze maintenu évite l'analyse FFT de l'entrée. La dernière fenêtre reste disponible pour reconstruire exactement l'historique nécessaire à une recapture entre deux pas d'analyse. La synthèse spectrale continue à sa cadence d'origine. Les tables FFT sont communes en lecture seule et préparées avant lecture.

Les historiques restent préalloués : un changement de type dans le callback n'alloue pas de mémoire. Le dimensionnement mémoire à huit emplacements reste un travail distinct.

## Protocole reproductible

`bench_audio` mesure des blocs de 64/128/256 échantillons à 44,1/48/96 kHz avec `--full`. Sept scénarios : vide, résonateur + deux granulaires, ajout du semi-modulaire, quatre résonateurs, FM + résonateur + granulaire + freeze + semi-modulaire (FM est la source), changements de types/ordre, bascule avec l'ancien moteur. Accords de 1/8/16 notes, relâchements et redéclenchements ; les queues peuvent porter le nombre de voix actives à 16 pour un accord de huit notes. Les champs `chord_notes` et `active_voices_peak` distinguent ces quantités.

Les mouvements de brillance et de taille des grains sont déterministes. Le temps mesuré contient les événements MIDI, la mise à jour des contrôles et le rendu stéréo. La préparation est exclue. Le dump optionnel est écrit après le chronométrage du bloc. Les temps CPU et leur granularité dépendent de l'OS ; les temps muraux p95/p99/max comprennent les interruptions du système.

Exemple CMake, puis exécutables dans le dossier de configuration selon le générateur :

```
cmake -S . -B build-perf -DCMAKE_BUILD_TYPE=Release
cmake --build build-perf --config Release --target bench_audio test_realtime
bench_audio --full > performance.csv
bench_audio --scenario resonator_two_granular > performance-granulaire.csv
```

La livraison Windows contient `Diagnostics/Mesurer_Performances.cmd`, `bench_audio.exe` et un guide. Le workflow publie aussi son propre CSV. Ces mesures hors hôte ne couvrent pas le pilote, les autres plugins, l'interface ni la priorité audio réelle. Elles ne garantissent pas l'absence de craquements dans Ableton ou un autre hôte.

## Comparaison dans le même processus

Des exécutions éloignées sur une machine partagée donnent parfois des temps contradictoires. `bench_compare` alterne les callbacks de l'ancienne version et de la nouvelle, en inversant le premier exécuté à chaque bloc. Il compare également les échantillons exactement, hors chronométrage. Il est optionnel : fournir `-DANKS_REFERENCE_SOURCE=/chemin/vers/le/checkout-reference` à CMake. Le checkout doit contenir `src/Engine.h` et les autres en-têtes de la référence. Cible : `bench_compare`.

Référence : commit `f70fc7646debcc5f14044b1a9350045792247b2c`. Comparaison Linux/GCC `-O2 -DNDEBUG`, 48 kHz, accords de 16 notes, contrôles en mouvement ; machine partagée. Le CSV complet est `performance/linux_paired_16_notes.csv`. Les chiffres ci-dessous sont un relevé, pas une vitesse garantie ni une moyenne entre machines.

| Chaîne, bloc de 128 | CPU moyen avant → après | Pire bloc de déclenchement avant → après |
|---|---:|---:|
| Résonateur + 2 granulaires | 1,230 → 1,074 ms | 5,396 → 0,631 ms |
| Résonateur + 2 granulaires + semi-modulaire | 1,570 → 1,422 ms | 6,864 → 0,867 ms |
| 4 résonateurs | 1,035 → 1,043 ms | 5,984 → 1,096 ms |
| Source FM, résonateur + granulaire + freeze + semi-modulaire | 2,507 → 1,966 ms | 5,736 → 1,181 ms |

Le coût moyen de quatre résonateurs est pratiquement inchangé dans ce relevé ; le gain concerne son pic de déclenchement. Le délai d'un bloc 128 à 48 kHz est 2,667 ms. Des blocs dépassent encore ce délai dans les charges extrêmes : notamment le p99 du cas FM/freeze, à environ 3,728 ms après optimisation. Cette révision ne valide donc pas toutes les combinaisons à 16 voix et petit buffer.

## Rendu et vérifications

- Comparaison complète sur 189 combinaisons de scénario/fréquence/buffer/accord : 11 850 300 trames stéréo identiques bit à bit sur ce build Linux. SHA-256 commun des dumps : `1b227b796605600739e1e8e2b0d5c8c14a5d252883f899902519661a8fb47c55`. Les CSV complets sont conservés à titre de traces ; leurs temps muraux proviennent d'exécutions distinctes et ne servent pas à annoncer les gains ci-dessus.
- Comparaison séparée du freeze avec maintien, relâchement et recapture très rapide, plus remise à zéro.
- Régressions core, sequencer, rack, effects, creative, FM et banque de 36 WAV.
- `test_realtime` : nouvelle note identique à une voix fraîche pour les neuf types, pas de fuite gauche/droite avec les coefficients partagés, recapture spectrale d'un sinus sans changement de phase, aucune allocation pendant les rafales MIDI et les changements de topologie.
- Détecteurs d'erreurs mémoire sur les tests créatifs. Les tests JUCE et le build Windows restent une validation séparée dans Actions.

Le test Windows de la source FM comparait précédemment un paramètre quantifié par pas de `0.001f` au littéral entier 7. Ce pas peut produire `7.00000048`. Le test compare désormais exactement la valeur réelle avant/après la recette ; il vérifie toujours que la recette ne touche pas à la source. Les exceptions du test du processeur sont imprimées avant son échec pour que le diagnostic soit visible.
