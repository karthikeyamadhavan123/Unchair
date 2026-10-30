#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TOKEN_STREAM_SIZE 4096
#define LITERAL_SIZE 1000
#define ERROR_MESSAGE_SIZE 100
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// remaining clear the memory by strndup later as it allocates using malloc.
typedef struct
{
  int col;
  int line;
} Position;

typedef enum TokenType
{
  LBRACKET,    // 0
  RBRACKET,    // 1
  COMMA,       // 2
  EQUALS,      // 3
  DOUBLEQUOTE, // 4
  INTEGER,     // 5
  STRING,      // 6
  KEYWORD      // 7
} TType;

/*
 example of Token
 LBRACKET,"["
*/

typedef struct
{
  TType t_type;
  char *value;
} Token;

/*
 this is the stream of tokens
 so it will be array of tokens.
 [{LBRACKET,"["},{RBRACKET,"]"}]
*/

typedef struct
{
  Token *tokens;
  int index; // tracks the array of tokens stream;
  int size;
} TokenStream;

typedef struct
{
  int work_minutes;
  int break_minutes;
  int min_work_minutes;
  int max_work_minutes;
  int adjust_step;
  int snooze_minutes;
  int max_snoozes;
  int water_minutes;
  int song_seconds;
  char *song_order; // Alternatively: char song_order[32];
} Config;

typedef struct
{
  int statusCode;
  char errorMessage[ERROR_MESSAGE_SIZE];
} Error;

const char *keys[] = {"work_minutes", "break_minutes", "min_work_minutes",
                      "max_work_minutes", "adjust_step", "snooze_minutes",
                      "max_snoozes", "water_minutes",
                      "song_seconds", "song_order"};

Position *InitializePostion()
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

TokenStream *InitializeStream()
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

Config *InitializeConfig()
{

  Config *c = (Config *)calloc(1, sizeof(Config));

  if (c == NULL)
  {
    printf("Config is not initialized.\n");
    return NULL;
  }

  return c;
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

TType *Peek(TokenStream *tokenStream, int *idx)
{
  if (*idx >= tokenStream->size)
  {
    return NULL;
  }
  return &tokenStream->tokens[*idx].t_type;
}

TType GetExpectedValueType(char *key)
{
  if (strcmp(key, "song_order") == 0)
  {
    return STRING;
  }
  return INTEGER;
}

int ParseAssignment(TokenStream *tokenStream, int *idx, Config *config)
{
  int keyIdx = *idx;
  TType *currVal = Peek(tokenStream, idx);
  if (!currVal)
  {
    printf("Error: Unexpected end of stream while parsing assignment.\n");
    return 0;
  }
  // Check if we hit the closing bracket
  if (*currVal == RBRACKET)
  {
    printf("Error: Configuration block cannot be empty.\n");
    return 0; // Fails and exits with 1, satisfying Test 14
  }
  if (currVal && *currVal == KEYWORD)
  {
    (*idx)++;
  }
  else
  {
    printf("Error: Expected a KEYWORD, but received token type %s.\n", tokenStream->tokens[keyIdx].value);
    return 0;
  }
  TType *assignOp = Peek(tokenStream, idx);
  if (assignOp && *assignOp == EQUALS)
  {
    (*idx)++;
  }
  else
  {
    printf("Error: Expected an EQUALS operator, but received %s.\n", tokenStream->tokens[*idx].value);
    return 0;
  }
  // Expecting a value (INTEGER or STRING)
  int valIdx = *idx;
  TType *value = Peek(tokenStream, idx);
  if (value && (*value == INTEGER || *value == STRING))
  {
    (*idx)++;
  }
  else
  {
    printf("Error: Expected an INTEGER or STRING value, but received %s.\n", tokenStream->tokens[valIdx].value);
    return 0;
  }
  // check the Type of the keyword;
  TType key_type = GetExpectedValueType(tokenStream->tokens[keyIdx].value);
  if (value && *value != key_type)
  {
    printf("Expected type is not matched for the Keyword %s\n.", tokenStream->tokens[keyIdx].value);
    return 0;
  }
  // here u can assign the configuration logic before checking for a comma.
  SetConfigKeys(tokenStream->tokens[keyIdx].value, tokenStream->tokens[valIdx].value, config);
  // Check for a comma or closing bracket next
  TType *nextVal = Peek(tokenStream, idx);
  if (!nextVal)
  {
    printf("Error: Unexpected end of stream after value assignment.\n");
    return 0;
  }

  if (*nextVal == COMMA)
  {
    (*idx)++;
    TType *expectKeyword = Peek(tokenStream, idx);
    if (!expectKeyword)
    {
      printf("Error: Unexpected end of stream after value assignment.\n");
      return 0;
    }
    if (*expectKeyword != KEYWORD)
    {
      printf("Error: Unexpected end of stream after value assignment , .\n");
      return 0;
    }
    return ParseAssignment(tokenStream, idx, config);
  }
  else if (*nextVal == RBRACKET)
  {
    (*idx)++;
    return 1;
  }
  else
  {
    printf("Error: Expected a COMMA or ']', but received token type %s.\n", tokenStream->tokens[*idx].value);
    return 0;
  }
}

// takes the assignment list and starts parsing
int ParseAssignments(TokenStream *tokenStream, int *idx, Config *config)
{
  TType *val = Peek(tokenStream, idx);
  if (!val)
  {
    printf("Error: Empty token stream or unexpected end.\n");
    return 0;
  }

  if (*val == LBRACKET)
  {
    (*idx)++; // Consume '['
    // Loop or call assignment parsing.
    // If ParseAssignment handles inner tokens and stops at ']', we call it:
    if (!ParseAssignment(tokenStream, idx, config))
    {
      return 0;
    }
  }
  else
  {
    printf("Error: Expected '[' at the start of assignments.\n");
    return 0;
  }

  // After parsing the assignment block, ensure we actually consumed everything up to tokenStream->size
  if (*idx < tokenStream->size)
  {
    printf("Error: Extra tokens present after closing ']'.\n");
    return 0;
  }
  return 1;
}

// parses tokens
int ParseTokens(TokenStream *tokenStream, Config *config)
{
  if (tokenStream->size == 0)
  {
    // this case is acceptable that's why we return true
    printf("No tokens in the stream.\n");
    return 1;
  }
  int p_index = 0;
  int res = ParseAssignments(tokenStream, &p_index, config);
  if (res == 0)
  {
    return 0;
  }
  return 1;
}

int main()
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
    return 1;
  }

  Config *config = InitializeConfig();

  if (config == NULL)
  {
    return 1;
  }

  FILE *fp = fopen(configPath, "r");

  if (fp == NULL)
  {
    printf("Error: Unable to open file 'unchair.un'.\n");
    free(tokenStream->tokens);
    free(tokenStream);
    free(position);
    return 1;
  }

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

      if (!isValidCharLiteral(nextCh))
      {
        printf("Error: Invalid character '%c' inside string literal at %d on line number %d.\n", nextCh, position->col, position->line);
        fclose(fp);
        CleanupTokens(tokenStream);
        free(tokenStream->tokens);
        free(tokenStream);
        free(position);
        return 1;
      }

      int literal_index = 0;

      if (nextCh == '"')
      {
        // means a empty string in our case we will not allow it.
        position->col++;
        countDoubleQuote--;
        printf("Error: Property keys and values cannot be empty at %d on line number %d.\n", position->col, position->line);
        fclose(fp);
        CleanupTokens(tokenStream);
        free(tokenStream->tokens);
        free(tokenStream);
        free(position);
        return 1;
      }

      else
      {
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
            fclose(fp);
            CleanupTokens(tokenStream);
            free(tokenStream->tokens);
            free(tokenStream);
            free(position);
            return 1;
          }

          if (literal_index >= LITERAL_SIZE)
          {
            printf("Exceeded string literal at position %d on line number %d.\n", position->col, position->line);
            fclose(fp);
            CleanupTokens(tokenStream);
            free(tokenStream->tokens);
            free(tokenStream);
            free(position);
            return 1;
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
          fclose(fp);
          CleanupTokens(tokenStream);
          free(tokenStream->tokens);
          free(tokenStream);
          free(position);
          return 1;
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
            fclose(fp);
            CleanupTokens(tokenStream);
            free(tokenStream->tokens);
            free(tokenStream);
            free(position);
            return 1;
          }
        }

      } while (1);
      numIndex = 0;
    }
    else
    {
      printf("Error: Unidentified character '%c' at position %d on line number %d.\n", ch, position->col, position->line);
      fclose(fp);
      CleanupTokens(tokenStream);
      free(tokenStream->tokens);
      free(tokenStream);
      free(position);
      return 1;
    }

  } while (1);

  if (countDoubleQuote > 0)
  {
    printf("Error: Unterminated string literal in config file at %d on line number %d.\n", position->col, position->line);
    fclose(fp);
    CleanupTokens(tokenStream);
    free(tokenStream->tokens);
    free(tokenStream);
    free(position);
    return 1;
  }

  printf("col position is %d\n", position->col);
  printf("Line lexed is %d\n", position->line);
  fclose(fp);
  // GetTokensFromStream(tokenStream);
  // call parser here after closing the file.
  int res = ParseTokens(tokenStream, config);
  if (res == 0)
  {
    CleanupTokens(tokenStream);
    free(tokenStream->tokens);
    free(tokenStream);
    free(position);
    if (config->song_order != NULL)
    {
      free(config->song_order);
    }
    free(config);
    return 1;
  }
  // printf("--- Timer Configuration ---\n");
  // printf("work_minutes     = %d\n", config->work_minutes);
  // printf("break_minutes    = %d\n", config->break_minutes);
  // printf("min_work_minutes = %d\n", config->min_work_minutes);
  // printf("max_work_minutes = %d\n", config->max_work_minutes);
  // printf("adjust_step      = %d\n", config->adjust_step);
  // printf("snooze_minutes   = %d\n", config->snooze_minutes);
  // printf("max_snoozes      = %d\n", config->max_snoozes);
  // printf("water_minutes    = %d\n", config->water_minutes);
  // printf("song_seconds     = %d\n", config->song_seconds);
  // printf("song_order       = %s\n", config->song_order);

  int width, height, channels;
  const char *imagePath = "../reactor.png";
  unsigned char *img = stbi_load(imagePath, &width, &height, &channels, 0);
  printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", width, height, channels);
  if (img == NULL)
  {
    printf("Error: Failed to load image %s\n", imagePath);
    CleanupTokens(tokenStream);
    free(tokenStream->tokens);
    free(tokenStream);
    free(position);
    if (config->song_order != NULL)
    {
      free(config->song_order);
    }
    free(config);
    return 1;
  }
  // Cleanup resources
  // fixed the strndup issue here
  CleanupTokens(tokenStream);
  free(tokenStream->tokens);
  free(tokenStream);
  free(position);
  if (config->song_order != NULL)
  {
    free(config->song_order);
  }
  free(config);
  stbi_image_free(img);
  return 0;
}