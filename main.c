#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TOKEN_SIZE 4096
#define TOKEN_STREAM_SIZE 4096
#define LITERAL_SIZE 1000
#define ERROR_MESSAGE_SIZE 100

typedef struct
{
  int curr;
  int line;
} Position;

typedef enum TokenType
{
  LBRACE,         // 0
  RBRACE,         // 1
  COMMA,          // 2
  EQUALS,         // 3
  DOUBLEQUOTE,    // 4
  NUMBER_LITERAL, // 5
  STRING_LITERAL, // 6
  KEYWORD         // 7
} TType;

/*
 example of Token
 LBRACE,"["
*/
typedef struct
{
  TType t_type;
  char *value;
} Token;

/*
 this is the stream of tokens
 so it will be array of tokens.
 [{LBRACE,"["},{RBRACE,"]"}]
*/

typedef struct
{
  Token *tokens;
  int index; // tracks the array of tokens stream;
  int size;
} TokenStream;

typedef struct
{
  int statusCode;
  char errorMessage[ERROR_MESSAGE_SIZE];
} Error;

const char *keys[] = {"work_minutes", "break_minutes", "min_work_minutes",
                      "max_work_minutes", "adjust_step", "snooze_minutes",
                      "max_snoozes", "water_minutes", "empty",
                      "song_seconds", "song_order"};

Position *InitializePostion(int curr, int line)
{
  if (curr < 0)
  {
    printf("Initializer current position is negative please check it.\n");
    return NULL;
  }
  if (line < 0)
  {
    printf("Initializer line position is negative please check it.\n");
    return NULL;
  }

  Position *p = (Position *)malloc(sizeof(Position));

  if (p == NULL)
  {
    printf("Position is not initialized.\n");
    return NULL;
  }

  p->curr = 0;
  p->line = 0;

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
    printf("token type is : %d\n", tokenStream->tokens[i].t_type);
    printf("value is %s\n", tokenStream->tokens[i].value);
  }
}

void Push(TokenStream *tokenStream, Token T)
{
  if (tokenStream->size >= TOKEN_STREAM_SIZE)
  {
    printf("Token stream size is hit,cant push others for memory preventing buffer overflow.\n");
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
    printf("Literal is NULL\n");
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

int isNotValidCharForNumberLiteral(char ch)
{
  if (ch == '.')
  {
    printf("decimal types not allowed\n");
    return 0;
  }
  return 1;
}

int main()
{

  Position *position = InitializePostion(1, 1);
  if (position == NULL)
  {
    return 1;
  }

  TokenStream *tokenStream = InitializeStream();

  if (tokenStream == NULL)
  {
    return 1;
  }

  FILE *fp = fopen("unchair.un", "r");

  if (fp == NULL)
  {
    printf("Error: Unable to open file 'unchair.un'.\n");
    free(tokenStream);
    free(position);
    return 1;
  }

  int countDoubleQuote = 0;
  int isValidToken = 1;
  do
  {
    // Taking input single character at a time
    char ch = fgetc(fp);
    Token t;
    char literal[LITERAL_SIZE];
    char numLiteral[LITERAL_SIZE];
    // Checking for end of file
    if (feof(fp))
      break;

    if (isSpace(ch) == 1)
    {
      continue;
    }
    if (ch == '[')
    {
      t = (Token){LBRACE, "["};
      Push(tokenStream, t);
      // position->curr++;
      // position->line++;
    }
    else if (ch == ']')
    {
      t = (Token){RBRACE, "]"};
      Push(tokenStream, t);
    }
    else if (ch == ',')
    {
      t = (Token){COMMA, ","};
      Push(tokenStream, t);
      // position->curr++;
      // position->line++;
    }
    else if (ch == '"')
    {
      // position->curr++;
      countDoubleQuote++;
      char nextCh = fgetc(fp); // u go to the next character;
      if (isNum(nextCh))
      {
        printf("Number is not allowed inside double quotes as variable name at "
               "position %d line number %d.\n",
               position->curr, position->line);
        return 1;
      }
      int index = 0;
      if (nextCh == '"')
      {
        // means a empty string in our case we will allow it.
        // position->curr++;
        countDoubleQuote--;
        literal[index] = '\0';
        index = 0;
        t = (Token){STRING_LITERAL, strndup(literal, LITERAL_SIZE)};
        Push(tokenStream, t);
      }
      else
      {
        // position->curr++;
        literal[index++] = nextCh;
        do
        {
          char c = fgetc(fp);
          // printf("characters after first inverted comma %c\n", c);
          if (feof(fp))
            break;
          if (c == '"')
          {
            countDoubleQuote--;
            literal[index] = '\0';
            index = 0;
            if (isKeyword(literal) == 1)
            {
              t = (Token){KEYWORD, strndup(literal, LITERAL_SIZE)};
              Push(tokenStream, t);
            }
            else
            {
              t = (Token){STRING_LITERAL, strndup(literal, LITERAL_SIZE)};
              Push(tokenStream, t);
            }
            break;
          }
          if(c==','){
            printf("character reached until comma %c\n",c);
            if(countDoubleQuote > 0){
              printf("missing double quote in the current line\n");
              break;
            }
          }
          // position->curr++;
          literal[index++] = c;
        } while (1);
      }
    }
    else if (ch == '=')
    {
      // position->curr++;
      t = (Token){EQUALS, "="};
      Push(tokenStream, t);
    }
    else if (isNum(ch))
    {
      // position->curr++;
      int numIndex = 0;
      numLiteral[numIndex++] = ch;
      // will not allow spaces between number.and no decimal number.
      do
      {
        char character = fgetc(fp);
        if (feof(fp))
          break;
        if (isSpace(character) == 1)
        {
          continue;
        }
        if (isNotValidCharForNumberLiteral(character) == 0)
        {
          printf("character is wrong %c\n", character);
          isValidToken = 0;
          break;
        }

        if (isNum(character))
        {
          // position->curr++;
          numLiteral[numIndex++] = character;
        }
        if (character == ',')
        {
          // position->curr++;
          // position->line++;
          numLiteral[numIndex] = '\0';
          t = (Token){NUMBER_LITERAL, strdup(numLiteral)};
          Push(tokenStream, t);
          t = (Token){COMMA, ","};
          Push(tokenStream, t);
          break;
        }
        if (character == ']')
        {
          numLiteral[numIndex] = '\0';
          t = (Token){NUMBER_LITERAL, strdup(numLiteral)};
          Push(tokenStream, t);
          t = (Token){RBRACE, "]"};
          Push(tokenStream, t);
          break;
        }

      } while (1);
      numIndex = 0;
    }
    else
    {
      printf("Unreconizable character %c\n", ch);
      isValidToken = 0;
      break;
    }

  } while (1);

  if (countDoubleQuote > 0)
  {
    printf("Please check your config file you have a missing double quote\n");
    return 1;
  }

  if (isValidToken == 0)
  {
    return 1;
  }
  GetTokensFromStream(tokenStream);
  // printf("curr position is %d\n", position->curr);
  // printf("Line lexed is %d\n", position->line);
  // Cleanup resources
  fclose(fp);
  free(tokenStream->tokens);
  free(tokenStream);
  free(position);

  return 0;
}