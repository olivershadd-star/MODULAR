#include "FactorySources.h"
#include "FactoryBinary.h"
namespace anks::factory {
struct Item{const char*name;const char*resource;int root;const char*category="Classiques";};
static constexpr Item items[]={
 {"Clap_Triplet.wav","Clap_Triplet_wav",60},
 {"FM_C2.wav","FM_C2_wav",36},
 {"Friction_Rugueuse.wav","Friction_Rugueuse_wav",60},
 {"Grain_Numerique.wav","Grain_Numerique_wav",60},
 {"Impact_Sec.wav","Impact_Sec_wav",60},
 {"Kick_Pulse.wav","Kick_Pulse_wav",35},
 {"Lame_110Hz.wav","Lame_110Hz_wav",45},
 {"Metal_Cluster.wav","Metal_Cluster_wav",60},
 {"Snare_Poudre.wav","Snare_Poudre_wav",60},
 {"Souffle_Continu.wav","Souffle_Continu_wav",60},
 {"Verre_Aigu.wav","Verre_Aigu_wav",72},
 {"Voyelle_Synthetique.wav","Voyelle_Synthetique_wav",45},
 {"Bois_Creux.wav","Bois_Creux_wav",48,"Bois et cordes"},
 {"Corde_Pincee.wav","Corde_Pincee_wav",45,"Bois et cordes"},
 {"Kalimba_Douce.wav","Kalimba_Douce_wav",60,"Bois et cordes"},
 {"Marimba_Fibre.wav","Marimba_Fibre_wav",48,"Bois et cordes"},
 {"Corde_Frottee.wav","Corde_Frottee_wav",55,"Bois et cordes"},
 {"Bambou_Pulse.wav","Bambou_Pulse_wav",60,"Bois et cordes"},
 {"Cloche_Felee.wav","Cloche_Felee_wav",60,"Metaux"},
 {"Plaque_Vibrante.wav","Plaque_Vibrante_wav",48,"Metaux"},
 {"Ressort_Metallique.wav","Ressort_Metallique_wav",55,"Metaux"},
 {"Bol_Chantant.wav","Bol_Chantant_wav",57,"Metaux"},
 {"Carillon_Mobile.wav","Carillon_Mobile_wav",72,"Metaux"},
 {"Lamelle_Rouillee.wav","Lamelle_Rouillee_wav",45,"Metaux"},
 {"Papier_Froisse.wav","Papier_Froisse_wav",60,"Textures organiques"},
 {"Sable_Roulant.wav","Sable_Roulant_wav",60,"Textures organiques"},
 {"Brosse_Granuleuse.wav","Brosse_Granuleuse_wav",60,"Textures organiques"},
 {"Gouttes_Resonantes.wav","Gouttes_Resonantes_wav",60,"Textures organiques"},
 {"Souffle_Anche.wav","Souffle_Anche_wav",50,"Textures organiques"},
 {"Crepitement_Doux.wav","Crepitement_Doux_wav",60,"Textures organiques"},
 {"FM_Evolutive.wav","FM_Evolutive_wav",36,"Matieres numeriques"},
 {"Impulsions_Irisees.wav","Impulsions_Irisees_wav",60,"Matieres numeriques"},
 {"Spectre_Mouvant.wav","Spectre_Mouvant_wav",48,"Matieres numeriques"},
 {"Voyelle_Morphing.wav","Voyelle_Morphing_wav",45,"Matieres numeriques"},
 {"Pixels_Fondants.wav","Pixels_Fondants_wav",60,"Matieres numeriques"},
 {"Nuage_Harmonique.wav","Nuage_Harmonique_wav",55,"Matieres numeriques"},
};
int count(){return int(sizeof(items)/sizeof(items[0]));}
const char*name(int i){return i>=0&&i<count()?items[i].name:"";}
const char*category(int i){return i>=0&&i<count()?items[i].category:"";}
int root(int i){return i>=0&&i<count()?items[i].root:60;}
const void*bytes(int i,int&size){size=0;return i>=0&&i<count()?ANKSFactory::getNamedResource(items[i].resource,size):nullptr;}
}
