#include <stdio.h>
#include <stdlib.h>
#define SIZE 4096
#define TOKEN_SIZE 4096
#define TOKEN_STREAM_SIZE 4096

typedef struct
{
    int curr;
    int line;
} Position;

typedef enum TokenType
{
    LBRACE,
    RBRACE,
    COMMA,
    EQUALS,
    DOUBLEQUOTE,
    IDENTIFIER,
    NUMBER,
    STRING
} TType;

/*
 example of Token
 LBRACE,"["
*/

typedef struct
{
    TType t_type;
    char value[TOKEN_SIZE];
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

int main()
{

    Position *position = InitializePostion(0, 0);
    TokenStream *tokenStream = InitializeStream();
    FILE *fp = fopen("unchair.txt", "r");

    if (fp == NULL)
    {
        printf("Error: Unable to open file 'unchair.un'.\n");
        free(position);
        return 1;
    }

    Token T = {LBRACE, "["};
    Token T1 = {RBRACE, "]"};
    Token T2 = {COMMA, ","};
    Token T3 = {EQUALS, "="};
    Token T4 = {DOUBLEQUOTE, "\""};
    Token T5 = {IDENTIFIER, "work_minutes\0"};
    Token T6 = {NUMBER, "345"};
    Token T7 = {STRING, "random\0"};

    tokenStream->tokens[++tokenStream->index] = T;
    tokenStream->size++;
    tokenStream->tokens[++tokenStream->index] = T1;
    tokenStream->size++;
    tokenStream->tokens[++tokenStream->index] = T2;
    tokenStream->size++;
    tokenStream->tokens[++tokenStream->index] = T3;
    tokenStream->size++;
    tokenStream->tokens[++tokenStream->index] = T4;
    tokenStream->size++;
    tokenStream->tokens[++tokenStream->index] = T5;
    tokenStream->size++;
    tokenStream->tokens[++tokenStream->index] = T6;
    tokenStream->size++;
    tokenStream->tokens[++tokenStream->index] = T7;
    tokenStream->size++;

    GetTokensFromStream(tokenStream);

    // Cleanup resources
    fclose(fp);
    free(tokenStream->tokens);
    free(tokenStream);
    free(position);

    return 0;
}