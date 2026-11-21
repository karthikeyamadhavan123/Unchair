#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

Config *InitializeConfig(void)
{

  Config *c = (Config *)calloc(1, sizeof(Config));

  if (c == NULL)
  {
    printf("Config is not initialized.\n");
    return NULL;
  }

  return c;
}

void SetConfigKeys(char *key, char *value, Config *config)
{

  if (strcmp("work_minutes", key) == 0)
  {
    config->work_minutes = atoi(value);
  }
  if (strcmp("break_minutes", key) == 0)
  {
    config->break_minutes = atoi(value);
  }
  if (strcmp("min_work_minutes", key) == 0)
  {
    config->min_work_minutes = atoi(value);
  }
  if (strcmp("max_work_minutes", key) == 0)
  {
    config->max_work_minutes = atoi(value);
  }
  if (strcmp("adjust_step", key) == 0)
  {
    config->adjust_step = atoi(value);
  }
  if (strcmp("snooze_minutes", key) == 0)
  {
    config->snooze_minutes = atoi(value);
  }
  if (strcmp("max_snoozes", key) == 0)
  {
    config->max_snoozes = atoi(value);
  }
  if (strcmp("water_minutes", key) == 0)
  {
    config->water_minutes = atoi(value);
  }
  if (strcmp("song_seconds", key) == 0)
  {
    config->song_seconds = atoi(value);
  }
  if (strcmp("song_order", key) == 0)
  {
    if (config->song_order != NULL)
    {
      free(config->song_order);
    }
    config->song_order = strndup(value, LITERAL_SIZE);
  }
}

void FreeConfig(Config *config)
{
  if (config == NULL)
  {
    return;
  }
  if (config->song_order != NULL)
  {
    free(config->song_order);
  }
  free(config);
}