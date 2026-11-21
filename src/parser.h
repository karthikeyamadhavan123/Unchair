#ifndef PARSER_H
#define PARSER_H

#include "types.h"

TType *Peek(TokenStream *tokenStream, int *idx);
TType GetExpectedValueType(char *key);

int ParseAssignment(TokenStream *tokenStream, int *idx, Config *config);
int ParseAssignments(TokenStream *tokenStream, int *idx, Config *config);
int ParseTokens(TokenStream *tokenStream, Config *config);

#endif /* PARSER_H */