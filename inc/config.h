#ifndef __CONFIG
#define __CONFIG
void config_init(char*);

int config_load();
int config_save();
int config_save_defaults();

int config_validate();

char  *config_get_string (char *);
double config_get_number (char *);
int    config_get_boolean(char *);
char  *config_get_secret (char *);
char  *config_get_button (int, char *);
#endif