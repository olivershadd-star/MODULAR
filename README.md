<div align="center">

# ANKS LABS / MODULAR
### HYBRID SYNTHESIS · RESONANCE · DSP LAB

**1.4 alpha 3 — six emplacements, treize traitements**

**BUILD. BREAK. REWIRE. LISTEN.**

[Sources](https://github.com/olivershadd-star/MODULAR) · [Toutes les versions](https://github.com/olivershadd-star/MODULAR/releases) · [Rapports de compilation](https://github.com/olivershadd-star/MODULAR/actions)

</div>

## ⬇ TÉLÉCHARGER ET INSTALLER — DOWNLOAD

[![TÉLÉCHARGER LA DERNIÈRE VERSION](https://img.shields.io/github/v/release/olivershadd-star/MODULAR?include_prereleases&sort=date&display_name=tag&style=for-the-badge&logo=github&logoColor=F7FAFF&label=T%C3%89L%C3%89CHARGER&labelColor=10151E&color=EF343E)](../../releases/latest) [![TOUTES LES VERSIONS](https://img.shields.io/badge/TOUTES_LES-VERSIONS-F5C343?style=for-the-badge&labelColor=10151E)](../../releases)

1. Clique sur **TÉLÉCHARGER**, puis, dans la rubrique *Assets*, télécharge `MODULAR-Windows-x64-X.Y.Z.zip` (X.Y.Z = numéro de version : prends toujours le plus grand).
2. Clic droit sur le ZIP → **Extraire tout**.
3. **Plugin pour Ableton** : copie le dossier `Modular.vst3` dans `C:\Program Files\Common Files\VST3`.
4. Dans Live : **Préférences → Plug-ins → Dossier système VST3 = On**, puis **Relancer l'analyse**.
5. **Sans Ableton** : lance `Standalone\Modular.exe`. Aucune installation. Détails dans `INSTALLATION.txt` (aussi dans le ZIP).

*Windows peut afficher « Windows a protégé votre ordinateur » : clique sur **Informations complémentaires → Exécuter quand même** (programme gratuit non signé, c'est normal).*

**EN —** Click **DOWNLOAD**, get `MODULAR-Windows-x64-X.Y.Z.zip` from *Assets*, extract it and copy `Modular.vst3` to `C:\Program Files\Common Files\VST3`, or run `Standalone\Modular.exe`.

---

Modular est un instrument C++17 / JUCE 8.0.6, VST3 et application autonome Windows x64. Une source unique alimente une chaîne de six traitements : WAV personnel, banque de 36 excitations, onde interne ou FM à deux opérateurs. Les modules traitent le signal reçu et n’ajoutent pas automatiquement une seconde source.

## Interface

Noir graphite, rouge profond, accents jaune mat et blanc cassé. Logo géométrique ANKS, circuits gravés discrets et potards à traits fins.

Une seule page affiche les six modules, les réglages de source, le séquenceur et le clavier. Les six cadres sont identiques, en grille fixe de deux colonnes et trois rangées. Tous les potards utilisent la même cellule de 70×88 dans le dessin de référence, quelle que soit la quantité de commandes du module. Les cases non utilisées restent libres. La page entière est mise à l’échelle uniformément pour entrer dans la fenêtre ; elle ne défile pas et les modules ne sont pas redimensionnés séparément.

La fenêtre s’ouvre au maximum à 960×640, ou plus petite selon l’écran. La barre de l’application autonome conserve **réduire, agrandir/restaurer et fermer**. Le clavier reste fixé en bas et la page est cadrée au-dessus, avec des marges pour empêcher tout recouvrement. Les panneaux secondaires FM et de détails gardent les mêmes potards et la même échelle que la page. Sur une petite fenêtre, tous les contrôles de la page diminuent ensemble.

Le compteur Audio indique le temps de calcul par rapport au budget du bloc. Pic et retards repartent de zéro à la préparation audio ; ce compteur ne mesure pas les pertes du pilote. Les modules OFF suspendent leur DSP après le fondu. ON repart avec une mémoire propre. Les raccords granulaires de l’alpha précédente sont conservés.

## Bibliothèque

| Traitement | Usage |
| --- | --- |
| Double corps | Deux corps résonants indépendants ou liés, balance, accord, brillance et position |
| Filtre / EQ | Filtre multimode modulé ou EQ quatre bandes |
| Saturation | Douce, dure ou wavefolding |
| Granulaire | Fragments capturés dans le signal entrant, hauteur, dispersion et densité |
| Delay | Échos avec feedback et couleur |
| Chorus | Trois voix modulées par canal |
| Décalage de fréquence | Déplacement signé en Hz, textures inharmoniques |
| Freeze spectral | Capture et maintien du spectre |
| Semi-modulaire | VCF, VCA, ADSR, LFO et deux routages de modulation, sans oscillateur ajouté |
| **Ring mod** | Métaux et battements par multiplication avec une sinusoïde ; ne produit aucun son sans entrée |
| **Bitcrusher** | Résolution 2–16 bits et maintien d’échantillons 1–64 ; aliasing et marches font partie de cet effet |
| **Phaser** | Six étages all-pass, LFO, profondeur, centre et feedback bipolaire |
| **Peigne / Comb** | Ligne accordée en Hz et feedback bipolaire, pour tubes et résonances |

Les quatre nouveaux effets sont des implémentations propres à Modular, sans code de firmware M-VAVE incorporé. Voir [l’étude FM‑1](docs/RECHERCHE_FM1.md) pour les moteurs candidats et leurs contraintes.

## Source et jeu

Importer un WAV ou choisir une excitation intégrée sélectionne la source WAV. FM sélectionne la source FM en amont de toute la chaîne. Les sources ne sont pas additionnées. La FM propose rapport, intensité, feedback, sensibilité à la vélocité et enveloppes de modulation/volume. Son suréchantillonnage 2× réduit l’aliasing sans l’éliminer aux réglages extrêmes.

Le séquenceur possède seize pas, vélocité et durée par pas. En autonome : Sequence, horloge Interne, PLAY. Dans un hôte : horloge Hôte et transport de l’hôte. Le menu de presets propose création complète, chaîne ou module A–F. Les créations complètes embarquent leur WAV ; les chaînes conservent la source et le séquenceur actuels.

## Compatibilité

Le nom du VST3 et de l’exécutable est **Modular**. Les codes VST3 `Anks` / `Arlc` sont conservés ; retirer l’ancien bundle du dossier scanné pour éviter deux produits avec la même identité. Les extensions de presets existantes sont conservées pour charger les fichiers précédents.

Les indices et identifiants des anciens paramètres restent en place, les nouveaux sont ajoutés à la fin. Les 24 anciens ordres bruts gardent leur sens, avec E/F vides à la restauration d’un ancien état. Les chaînes de schémas 1–3 remettent E/F à vide. Les listes de types et d’ordre sont étendues : **vérifier les automations enregistrées en valeurs normalisées**, dont l’interprétation peut changer dans l’hôte. Sauvegarder une copie des projets avant remplacement.

## Compilation et vérification

Lancer `Construire_Windows.ps1` avec CMake et Visual Studio C++/SDK Windows, ou le workflow **VST3 Windows**. JUCE 8.0.6 est épinglé à son commit. L’artifact **Modular-V1-4-Alpha-3-Windows** contient `Modular.vst3`, `Standalone/Modular.exe` et les diagnostics. Les rapports de tests, mesures et deux aperçus de l’interface sont publiés séparément.

Les tests couvrent le moteur, le séquenceur, les effets, la FM, le vrai bypass des treize traitements, les 720 ordres, la conservation des 24 anciens ordres, les mémoires stéréo, l’absence d’allocation pendant le rendu et les presets du plugin. Le banc inclut FM + six traitements mixtes et six résonateurs. Six emplacements ne garantissent pas six traitements lourds sans dépassement sur tout PC : le buffer, la polyphonie, les queues et les réglages comptent. Voir [les changements](docs/EVOLUTION_V1_4_ALPHA_3.md) et les protocoles dans `docs/PERFORMANCES.md`.
