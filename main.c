#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    NUMBER_LITERAL,
    STRING_LITERAL
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
    tokenStream->tokens[++tokenStream->index] = T;
    tokenStream->size++;
}

int isSpace(char ch)
{
    if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' || ch == '\v' || ch == '\f')
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

    FILE *fp = fopen("unchair.txt", "r");

    if (fp == NULL)
    {
        printf("Error: Unable to open file 'unchair.un'.\n");
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
        char literal[1000];
        if (literal == NULL)
        {
            printf("Literal Allocation failed.\n");
            return 1;
        }
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
            position->line++;
        }
        else if (ch == '"')
        {
            countDoubleQuote++;
            char nextCh = fgetc(fp); // u go to the next character;
            if (isNum(nextCh))
            {
                printf("Number is not allowed.\n");
                return 1;
            }
            int index = 0;
            if (nextCh == '"')
            {
                // means a empty string in our case we will allow it.
                countDoubleQuote--;
                literal[index] = '\0';
                index = 0;
                t = (Token){STRING_LITERAL, strdup(literal)};
                Push(tokenStream, t);
            }
            else
            {
                literal[index++] = nextCh;
                do
                {
                    char c = fgetc(fp);
                    if (feof(fp))
                        break;
                    if (isSpace(c) == 1)
                    {
                        continue;
                    }
                    if (c == '"')
                    {
                        countDoubleQuote--;
                        literal[index] = '\0';
                        index = 0;
                        t = (Token){STRING_LITERAL, strdup(literal)};
                        Push(tokenStream, t);
                        break;
                    }
                    literal[index++] = c;

                } while (1);
            }
        }
        else if (ch == '=')
        {
            t = (Token){EQUALS, "="};
            char nextChar = fgetc(fp);
            Push(tokenStream, t);
        }
        else if (isNum(ch))
        {
            // char numberLiteral[1000];
            printf("character : %c\n", (char)ch);
            // do
            // {
            //   if(feof(fp)) break;
            // } while (1);
        }
        else
        {
            printf("Unreconizable character %c!!!!\n", ch);
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