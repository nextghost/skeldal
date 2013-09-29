#ifndef SKELDAL_GAMESAVE
#define SKELDAL_GAMESAVE

char save_codelocks(PMEMFILE *fsta); //uklada do savegame nastaveni kodovych zamku (128 bytu);
char load_codelocks(PMEMFILE fsta, int *seekPos); //obnovuje ze savegame nastaveni kodovych zamku (128 bytu);
void save_enemy_paths(PMEMFILE *fsta);
int load_enemy_paths(PMEMFILE fsta, int *seekPos);
int save_spells(PMEMFILE *fsta);
int load_spells(PMEMFILE fsta, int *seekPos);
char save_dialog_info(PMEMFILE *fsta);
char load_dialog_info(PMEMFILE fsta, int *seekPos);

#endif