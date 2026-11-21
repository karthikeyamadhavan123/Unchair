#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "config.h"

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
  if (!assignOp)
  {
    printf("Error: Expected an EQUALS operator, but the stream ended.\n");
    return 0;
  }

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
  if (!value)
  {
    printf("Error: Expected an INTEGER or STRING value, but the stream ended.\n");
    return 0;
  }
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