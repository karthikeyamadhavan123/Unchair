#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include "types.h"

Position *InitializePostion(void);
TokenStream *InitializeStream(void);

void Push(TokenStream *tokenStream, Token T);
void GetTokensFromStream(TokenStream *tokenStream);
void CleanupTokens(TokenStream *tokenStream);
void FreeStream(TokenStream *tokenStream);

int isSpace(char ch);
int isNum(char ch);
int isKeyword(char *literal);
int isValidCharLiteral(char ch);

/* Lexes the whole file into tokenStream.
   Returns 1 on success, 0 on a lexing error (message already printed).
   The caller owns fp and all cleanup. */
int Lex(FILE *fp, TokenStream *tokenStream, Position *position);

#endif /* LEXER_H */