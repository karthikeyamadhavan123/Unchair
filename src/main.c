#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "lexer.h"
#include "parser.h"
#include "config.h"

int main(void)
{
  const char *configPath = "../unchair.un";
  Position *position = InitializePostion();
  if (position == NULL)
  {
    return 1;
  }

  TokenStream *tokenStream = InitializeStream();

  if (tokenStream == NULL)
  {
    free(position);
    return 1;
  }

  Config *config = InitializeConfig();

  if (config == NULL)
  {
    FreeStream(tokenStream);
    free(position);
    return 1;
  }

  FILE *fp = fopen(configPath, "r");

  if (fp == NULL)
  {
    printf("Error: Unable to open file 'unchair.un'.\n");
    FreeStream(tokenStream);
    free(position);
    FreeConfig(config);
    return 1;
  }

  int lexed = Lex(fp, tokenStream, position);
  fclose(fp);

  if (lexed == 0)
  {
    CleanupTokens(tokenStream);
    FreeStream(tokenStream);
    free(position);
    FreeConfig(config);
    return 1;
  }

  printf("col position is %d\n", position->col);
  printf("Line lexed is %d\n", position->line);
  // call parser here after closing the file.
  int res = ParseTokens(tokenStream, config);

  CleanupTokens(tokenStream);
  FreeStream(tokenStream);
  free(position);
  FreeConfig(config);

  if (res == 0)
  {
    return 1;
  }
  return 0;
}