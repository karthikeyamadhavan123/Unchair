#ifndef CONFIG_H
#define CONFIG_H

#include "types.h"

Config *InitializeConfig(void);
void SetConfigKeys(char *key, char *value, Config *config);
void FreeConfig(Config *config);

#endif /* CONFIG_H */