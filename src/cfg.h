#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef uint32_t DWORD;

#define TOKEN_EOF    0
#define TOKEN_WORD   'A'
#define TOKEN_STRING '"'
#define TOKEN_EQUALS '='
#define TOKEN_LBRACE '{'
#define TOKEN_RBRACE '}'

typedef struct AWM_Key
{
        char   *Name;
        char   *Value;
        struct AWM_Key *Next, *Child, *Parent;
} AWM_Key;

AWM_Key *AWM_ReadConfig(const char *Path);
void     AWM_FreeConfig(AWM_Key *Root);
AWM_Key *AWM_ConfigFetch(AWM_Key *Root, const char *Path);
DWORD    AWM_ConfigFetchDword(AWM_Key *Root, const char *Path);
char   **AWM_ConfigSplitPath(const char *Path, int *count);
void     AWM_ConfigFreeSplitPath(char **parts, int count);
void     AWM_DisplayConfig(AWM_Key *Root);

#endif
