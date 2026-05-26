#include "cfg.h"
#include <string.h>
#include <ctype.h>
#include <assert.h>

char *AWM_ReadEntireFile(const char *Path)
{
        FILE *fp = fopen(Path, "r");
        if (!fp)
                return NULL;
        fseek(fp, 0, SEEK_END);
        unsigned long bytes = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        char *Data = calloc(1, bytes + 1);
        if (!Data)
                return NULL;
        fread(Data, 1, bytes, fp);
        fclose(fp);
        Data[bytes] = EOF;
        return Data;
}

AWM_Key *AWM_CreateKey(const char *Name, const char *Value)
{
        AWM_Key *key = (AWM_Key *)calloc(1, sizeof(AWM_Key));
        key->Name = strdup(Name);
        key->Value = Value ? strdup(Value) : NULL;
        key->Next = NULL;
        key->Child = NULL;
        return key;
}

void AWM_AddKeyToList(AWM_Key *Root, AWM_Key *NewKey)
{
        AWM_Key *current = Root;
        while (current->Next)
                current = current->Next;
        current->Next = NewKey;
}

int AWM_ConTokenise(char *Data, size_t *Position, char *Identifier, size_t *szIdentifier, size_t mxIdentifier)
{
        *szIdentifier = 0;
        memset(Identifier, 0, mxIdentifier);
        if (Data[*Position] == '\0')
                return TOKEN_EOF;

        char Chr = Data[*Position];
        while (isspace(Chr))
        {
                (*Position)++;
                Chr = Data[*Position];
        }

        if (Chr == '/' && Data[*Position + 1] == '*')
        {
                (*Position) += 2;
                while (1)
                {
                        Chr = Data[*Position];
                        if (Chr == '\0')
                                break;
                        if (Chr == '*' && Data[*Position + 1] == '/')
                        {
                                (*Position) += 2;
                                break;
                        }
                        (*Position)++;
                }
                return AWM_ConTokenise(Data, Position, Identifier, szIdentifier, mxIdentifier);
        }

        if (Chr == '\0')
                return TOKEN_EOF;

        if (Chr == '"' || Chr == '\'')
        {
                char QuoteType = Chr;
                (*Position)++;
                Chr = Data[*Position];
                while (Chr != QuoteType && Chr != '\0' && *szIdentifier < mxIdentifier - 1)
                {
                        Identifier[(*szIdentifier)++] = Chr;
                        (*Position)++;
                        Chr = Data[*Position];
                }
                if (Chr == QuoteType)
                        (*Position)++;
                Identifier[*szIdentifier] = '\0';
                return TOKEN_STRING;
        }
        else if (isdigit(Chr) || Chr == '-')
        {
                while ((isdigit(Chr) || Chr == '-' || isalpha(Chr)) && *szIdentifier < mxIdentifier - 1)
                {
                        Identifier[(*szIdentifier)++] = Chr;
                        (*Position)++;
                        Chr = Data[*Position];
                }
                Identifier[*szIdentifier] = '\0';
                return TOKEN_STRING;
        }
        else if (isalpha(Chr) || isdigit(Chr) || Chr == '_' || Chr == '-')
        {
                while ((isalpha(Chr) || isdigit(Chr) || Chr == '_' || Chr == '-') && *szIdentifier < mxIdentifier - 1)
                {
                        Identifier[(*szIdentifier)++] = Chr;
                        (*Position)++;
                        Chr = Data[*Position];
                }
                Identifier[*szIdentifier] = '\0';
                return TOKEN_WORD;
        }
        else
        {
                (*Position)++;
                return Chr;
        }
}

AWM_Key *AWM_ConStatement(char *Data, size_t *Position, int *Token, AWM_Key *Parent)
{
        char Identifier[256] = {0};
        size_t SizeOfIdentifier = 0;

        *Token = AWM_ConTokenise(Data, Position, Identifier, &SizeOfIdentifier, sizeof(Identifier));

        if (*Token == TOKEN_EOF)
                return NULL;

        if (*Token == TOKEN_WORD)
        {
                AWM_Key *Key = calloc(1, sizeof(AWM_Key));
                if (!Key)
                        return NULL;

                Key->Name = strdup(Identifier);
                Key->Parent = Parent;
                Key->Next = NULL;
                Key->Child = NULL;
                Key->Value = NULL;

                int NextToken;
                char NextIdentifier[256] = {0};
                size_t NextSize = 0;
                size_t TempPos = *Position;
                NextToken = AWM_ConTokenise(Data, &TempPos, NextIdentifier, &NextSize, sizeof(NextIdentifier));
                if (NextToken == '=')
                {
                        *Position = TempPos;
                        int ValueToken;
                        char ValueStr[256] = {0};
                        size_t ValueSize = 0;
                        ValueToken = AWM_ConTokenise(Data, Position, ValueStr, &ValueSize, sizeof(ValueStr));
                        if (ValueToken == TOKEN_STRING)
                        {
                                Key->Value = strdup(ValueStr);
                        }

                        return Key;
                }
                else if (NextToken == '{')
                {
                        *Position = TempPos;
                        AWM_Key *Child = NULL;
                        AWM_Key *LastChild = NULL;

                        while (1)
                        {
                                int ChildToken;
                                AWM_Key *NewChild = AWM_ConStatement(Data, Position, &ChildToken, Key);

                                if (ChildToken == '}')
                                {
                                        break;
                                }

                                if (!NewChild)
                                        break;

                                if (!Child)
                                        Child = NewChild;
                                else
                                        LastChild->Next = NewChild;

                                LastChild = NewChild;
                        }

                        Key->Child = Child;
                        return Key;
                }
                else
                {
                        assert("bruh");
                        return Key;
                }
        }
        else if (*Token == TOKEN_STRING)
        {
                AWM_Key *Key = calloc(1, sizeof(AWM_Key));
                if (!Key)
                        return NULL;

                Key->Value = strdup(Identifier);
                Key->Name = strdup("");
                Key->Parent = Parent;

                *Token = AWM_ConTokenise(Data, Position, Identifier, &SizeOfIdentifier, sizeof(Identifier));
                return Key;
        }
        else if (*Token == '}')
        {
                return NULL;
        }

        return NULL;
}

AWM_Key *AWM_ReadConfig(const char *IniPath)
{
        char *Data = AWM_ReadEntireFile(IniPath);
        if (!Data)
                return NULL;

        AWM_Key *Root = NULL;
        AWM_Key *Last = NULL;
        size_t Position = 0;
        int Token = 0;

        while (1)
        {
                AWM_Key *New = AWM_ConStatement(Data, &Position, &Token, NULL);
                if (!New)
                        break;

                if (!Root)
                        Root = New;
                else
                        Last->Next = New;

                Last = New;
        }

        free(Data);
        return Root;
}

void AWM_FreeConfig(AWM_Key *Root)
{
        if (!Root)
                return;
        AWM_FreeConfig(Root->Child);
        AWM_FreeConfig(Root->Next);
        free(Root->Name);
        if (Root->Value)
                free(Root->Value);
        free(Root);
}

char **AWM_ConfigSplitPath(const char *Path, int *count)
{
        if (!Path || !count)
                return NULL;
        *count = 1;
        for (const char *p = Path; *p; p++)
        {
                if (*p == '/')
                        (*count)++;
        }

        char **parts = (char **)malloc(sizeof(char *) * (*count));
        char *pathCopy = strdup(Path);
        char *token = strtok(pathCopy, "/");
        int i = 0;

        while (token && i < *count)
        {
                parts[i++] = strdup(token);
                token = strtok(NULL, "/");
        }

        free(pathCopy);
        return parts;
}

void AWM_ConfigFreeSplitPath(char **parts, int count)
{
        if (!parts)
                return;
        for (int i = 0; i < count; i++)
        {
                if (parts[i])
                        free(parts[i]);
        }
        free(parts);
}

AWM_Key *AWM_ConfigFetch(AWM_Key *Root, const char *Path)
{
        if (!Root || !Path)
                return NULL;

        int depth;
        char **parts = AWM_ConfigSplitPath(Path, &depth);
        if (!parts)
                return NULL;

        AWM_Key *current = Root;

        for (int i = 0; i < depth; i++)
        {
                if (!current)
                {
                        AWM_ConfigFreeSplitPath(parts, depth);
                        return NULL;
                }

                AWM_Key *found = NULL;
                if (current->Child)
                {
                        AWM_Key *child = current->Child;
                        while (child)
                        {
                                if (child->Name && strcmp(child->Name, parts[i]) == 0)
                                {
                                        found = child;
                                        break;
                                }
                                child = child->Next;
                        }
                }

                if (!found && i == 0)
                {
                        AWM_Key *sibling = current;
                        while (sibling)
                        {
                                if (sibling->Name && strcmp(sibling->Name, parts[i]) == 0)
                                {
                                        found = sibling;
                                        break;
                                }
                                sibling = sibling->Next;
                        }
                }

                if (!found)
                {
                        AWM_ConfigFreeSplitPath(parts, depth);
                        return NULL;
                }

                current = found;
        }

        AWM_ConfigFreeSplitPath(parts, depth);
        return current;
}

void AWM_DisplayConTree(AWM_Key *Root, int Depth)
{
        if (!Root)
                return;

        for (int i = 0; i < Depth; i++)
                printf("  ");
        if (Root->Value)
        {
                printf("%s = %s\n", Root->Name, Root->Value);
        }
        else
        {
                printf("%s {\n", Root->Name);
                if (Root->Child)
                        AWM_DisplayConTree(Root->Child, Depth + 1);
                for (int i = 0; i < Depth; i++)
                        printf("  ");
                printf("}\n");
        }
        if (Root->Next)
                AWM_DisplayConTree(Root->Next, Depth);
}

void AWM_DisplayConfig(AWM_Key *Root)
{
        if (!Root)
        {
                printf("(empty)\n");
                return;
        }
        AWM_DisplayConTree(Root, 0);
}

DWORD AWM_ConfigFetchDword(AWM_Key *Root, const char *Path)
{
        AWM_Key *Key = AWM_ConfigFetch(Root, Path);
        return strtoul(Key->Value, NULL, 0);
}
