#ifndef TYPES_H
#define TYPES_H

#define TOKEN_STREAM_SIZE 4096
#define LITERAL_SIZE 1000
#define ERROR_MESSAGE_SIZE 100

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

#endif /* TYPES_H */