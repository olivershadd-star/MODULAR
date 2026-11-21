# 1.3 alpha 4 — bypass et raccords granulaires

Deux défauts reproduits dans le moteur : les modules désactivés continuaient leur calcul après le fondu ; les fragments granulaires bouclaient avec une rupture possible entre fin et début.

OFF devient transparent puis suspend le DSP après environ 160 ms de fondu. Une nouvelle voix avec un module OFF ne lance pas ce traitement. ON réinitialise la mémoire et revient progressivement : les anciennes queues ne sont pas restaurées. Les identifiants de paramètres sont conservés.

Le granulaire raccorde progressivement la fin du fragment à son début, avec une zone limitée à 2 ms ou au quart du fragment. La capture reste disponible après environ 5 ms. La dispersion conserve au moins 5 ms de fragment quand la capture disponible et la longueur demandée le permettent. Le rendu change sur ces transitions : cette version ne revendique pas une identité échantillon par échantillon avec alpha 3.

Un test sur rampe reproduit une marche maximale de 0,398326 avant correction et 0,00334728 après. Ce cas synthétique ne prouve pas que tous les craquements observés sont résolus. Le test bypass couvre les neuf types, la transparence OFF et la reprise sans ancienne mémoire. Le banc ajoute une chaîne avec deux granulaires et un semi-modulaire OFF.

L’indicateur Audio donne le temps écoulé dans processBlock divisé par la durée du bloc. Pic et retards sont cumulés depuis la préparation audio. 100 % représente le budget du bloc ; les retards comptent les traitements qui dépassent ce budget. Ce compteur ne détecte ni les pertes du pilote, ni les retards avant l’entrée dans processBlock. Un compteur interne distinct suit les blocs rendus silencieux faute de verrou disponible. Les tests du plugin vérifient aussi ce chemin.

La vidéo ne permet pas à elle seule de conclure à une surcharge CPU. Le pilote, la taille du buffer et un essai de la nouvelle compilation restent nécessaires pour distinguer dépassements de calcul et défauts de signal. Les mesures alpha 3 restent historiques dans PERFORMANCES.md.
