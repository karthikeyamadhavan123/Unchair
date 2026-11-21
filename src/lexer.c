#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"

static const char *keys[] = {"work_minutes", "break_minutes", "min_work_minutes",
                             "max_work_minutes", "adjust_step", "snooze_minutes",
                             "max_snoozes", "water_minutes",
                             "song_seconds", "song_order"};

Position *InitializePostion(void)
{
  Position *p = (Position *)malloc(sizeof(Position));

  if (p == NULL)
  {
    printf("Position is not initialized.\n");
    return NULL;
  }

  p->col = 1;
  p->line = 1;

  return p;
}

TokenStream *InitializeStream(void)
{
  TokenStream *tokenStream = (TokenStream *)malloc(sizeof(TokenStream));

  if (tokenStream == NULL)
  {
    printf("Stream is not initialized.\n");
    return NULL;
  }
  tokenStream->tokens = (Token *)malloc(TOKEN_STREAM_SIZE * sizeof(Token));
  if (tokenStream->tokens == NULL)
  {
    printf("Tokens Stream is not initialized.\n");
    return NULL;
  }
  tokenStream->index = -1;
  tokenStream->size = 0;
  return tokenStream;
}

void GetTokensFromStream(TokenStream *tokenStream)
{
  if (tokenStream->size == 0)
  {
    printf("No tokens in the stream\n");
    return;
  }
  printf("Size is %d\n", tokenStream->size);
  for (int i = 0; i < tokenStream->size; i++)
  {
    printf("Token type is : %d\n", tokenStream->tokens[i].t_type);
    printf("Value is %s\n", tokenStream->tokens[i].value);
  }
}

void Push(TokenStream *tokenStream, Token T)
{
  if (tokenStream->size >= TOKEN_STREAM_SIZE)
  {
    printf("Token stream size is hit,cant push others for preventing buffer overflow.\n");
    return;
  }
  tokenStream->tokens[++tokenStream->index] = T;
  tokenStream->size++;
}

int isSpace(char ch)
{
  if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' || ch == '\v' ||
      ch == '\f')
  {
    return 1;
  }
  return 0;
}

int isNum(char ch)
{
  int res = ch - '0';
  if (res >= 0 && res <= 9)
  {
    return 1;
  }
  return 0;
}

int isKeyword(char *literal)
{
  if (literal == NULL)
  {
    printf("Empty string is being passed.\n");
    return -1;
  }

  int num_keys = sizeof(keys) / sizeof(keys[0]);

  for (int i = 0; i < num_keys; i++)
  {
    if (strcmp(literal, keys[i]) == 0)
    {
      return 1;
    }
  }
  return 0;
}

int isValidCharLiteral(char ch)
{
  // accept spaces, _ and a-z and A-Z only

  if ((ch >= 65 && ch <= 90) || (ch >= 97 && ch <= 122) || (ch == 32) || (ch == 95))
  {
    return 1;
  }

  return 0;
}

void CleanupTokens(TokenStream *tokenStream)
{
  for (int i = 0; i < tokenStream->size; i++)
  {
    if (tokenStream->tokens[i].t_type == INTEGER ||
        tokenStream->tokens[i].t_type == STRING ||
        tokenStream->tokens[i].t_type == KEYWORD)
    {
      if (tokenStream->tokens[i].value != NULL)
      {
        free(tokenStream->tokens[i].value);
      }
    }
  }
}

void FreeStream(TokenStream *tokenStream)
{
  if (tokenStream == NULL)
  {
    return;
  }
  free(tokenStream->tokens);
  free(tokenStream);
}

int Lex(FILE *fp, TokenStream *tokenStream, Position *position)
{
  int countDoubleQuote = 0;

  do
  {
    // Taking input single character at a time
    char ch = fgetc(fp);
    Token t;
    char stringLiteral[LITERAL_SIZE];
    char numLiteral[LITERAL_SIZE];

    // Checking for end of file
    if (feof(fp))
      break;

    if (isSpace(ch) == 1)
    {
      if (ch == '\n')
      {
        position->line++;
      }
      position->col++;
      continue;
    }
    if (ch == '[')
    {
      t = (Token){LBRACKET, "["};
      Push(tokenStream, t);
      position->col++;
      // position->line++;
    }
    else if (ch == ']')
    {
      t = (Token){RBRACKET, "]"};
      Push(tokenStream, t);
      position->col++;
    }
    else if (ch == ',')
    {
      t = (Token){COMMA, ","};
      Push(tokenStream, t);
      position->col++;
      // position->line++;
    }
    else if (ch == '"')
    {
      position->col++;
      countDoubleQuote++;
      // u go to the next character;
      char nextCh = fgetc(fp);

      int literal_index = 0;

      if (nextCh == '"')
      {
        // means a empty string in our case we will not allow it.
        position->col++;
        countDoubleQuote--;
        printf("Error: Property keys and values cannot be empty at %d on line number %d.\n", position->col, position->line);
        return 0;
      }

      else
      {
        if (!isValidCharLiteral(nextCh))
        {
          printf("Error: Invalid character '%c' inside string literal at %d on line number %d.\n", nextCh, position->col, position->line);
          return 0;
        }
        position->col++;
        stringLiteral[literal_index++] = nextCh;
        do
        {
          char c = fgetc(fp);

          if (feof(fp))
            break;

          if (c != '"' && !isValidCharLiteral(c))
          {
            printf("Error: Invalid character '%c' inside string literal at %d on line number %d.\n", c, position->col, position->line);
            return 0;
          }

          if (literal_index >= LITERAL_SIZE)
          {
            printf("Exceeded string literal at position %d on line number %d.\n", position->col, position->line);
            return 0;
          }

          if (c == '"')
          {
            position->col++;
            countDoubleQuote--;
            stringLiteral[literal_index] = '\0';
            literal_index = 0;
            if (isKeyword(stringLiteral) == 1)
            {
              t = (Token){KEYWORD, strndup(stringLiteral, LITERAL_SIZE)};
              Push(tokenStream, t);
            }
            else
            {
              t = (Token){STRING, strndup(stringLiteral, LITERAL_SIZE)};
              Push(tokenStream, t);
            }
            break;
          }
          // we are not checking for the index maximum limit
          position->col++;
          stringLiteral[literal_index++] = c;
        } while (1);
      }
    }
    else if (ch == '=')
    {
      t = (Token){EQUALS, "="};
      Push(tokenStream, t);
      position->col++;
    }
    else if (isNum(ch))
    {
      position->col++;
      int numIndex = 0;
      numLiteral[numIndex++] = ch;

      do
      {
        char character = fgetc(fp);
        if (feof(fp))
        {
          numLiteral[numIndex] = '\0';
          t = (Token){INTEGER, strdup(numLiteral)};
          Push(tokenStream, t);
          break;
        }
        // bounding of index
        if (numIndex >= LITERAL_SIZE)
        {
          printf("Exceeded number literal at position %d on line number %d.\n", position->col, position->line);
          return 0;
        }

        if (isNum(character))
        {
          position->col++;
          numLiteral[numIndex++] = character;
        }

        else
        {
          if (character == ',')
          {
            // position->line++;
            numLiteral[numIndex] = '\0';
            t = (Token){INTEGER, strdup(numLiteral)};
            Push(tokenStream, t);
            ungetc(',', fp);
            break;
          }
          if (character == ']')
          {
            numLiteral[numIndex] = '\0';
            t = (Token){INTEGER, strdup(numLiteral)};
            Push(tokenStream, t);
            ungetc(']', fp);
            break;
          }
          // after a number digit space exists so terminate the number literal and continue to the next one
          if (isSpace(character))
          {
            position->col++;
            numLiteral[numIndex] = '\0';
            t = (Token){INTEGER, strdup(numLiteral)};
            Push(tokenStream, t);
            break;
          }
          else
          {
            printf("Error: Unidentified character '%c' at position %d on line number %d.\n", character, position->col, position->line);
            return 0;
          }
        }

      } while (1);
      numIndex = 0;
    }
    else
    {
      printf("Error: Unidentified character '%c' at position %d on line number %d.\n", ch, position->col, position->line);
      return 0;
    }

  } while (1);

  if (countDoubleQuote > 0)
  {
    printf("Error: Unterminated string literal in config file at %d on line number %d.\n", position->col, position->line);
    return 0;
  }

  return 1;
}