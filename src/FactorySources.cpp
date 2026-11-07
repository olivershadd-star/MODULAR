#include "FactorySources.h"
#include "FactoryBinary.h"
namespace anks::factory {
struct Item{const char*name;const char*resource;int root;};
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
};
int count(){return int(sizeof(items)/sizeof(items[0]));}
const char*name(int i){return i>=0&&i<count()?items[i].name:"";}
int root(int i){return i>=0&&i<count()?items[i].root:60;}
const void*bytes(int i,int&size){size=0;return i>=0&&i<count()?ANKSFactory::getNamedResource(items[i].resource,size):nullptr;}
}
