/*
* History:
*   0.1     2017-01-6   Initial version
*
* Usage:
* 
*/

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef void (*jc_ini_handler)(const char* section, const char* key, const char* value, void* ctx);

int jc_ini_parse_from_path(const char* path, jc_ini_handler handler, void* ctx);
int jc_ini_parse_from_file(FILE* f, jc_ini_handler handler, void* ctx);



#if defined(JC_INI_IMPLEMENTATION)

// Prototypes
char* jc_ini_getline(FILE* f, size_t* size, char* line);
int jc_ini_parseline(char* line, char** key_or_section, char** value);

char* jc_ini_getline(FILE* f, size_t* size, char* line)
{
    size_t offset = 0;
    const char* result = 0;
    while( (result = fgets(&line[offset], (int)(*size - offset), f)) )
    {
        size_t len = strlen(line);
        if( line[len-1] != '\n' ) // The line was too long
        {
            offset = len;
            *size += BUFSIZ;
            line = (char*)realloc(line, *size);
            continue;
        }

        return line;
    }

    return 0;
}

int jc_ini_parseline(char* line, char** outkey, char** outvalue)
{
    *outkey = 0;
    *outvalue = 0;

    size_t len = strlen(line);

    char* end = line+len-1;
    while( isspace(*line) && line < end )
        line++;
    while( isspace(*end) && end > line )
        *end-- = 0;

    if( line == end || line[0] == ';' || line[0] == '#' )
        return 0;

    if( line[0] == '[' )
    {
        while( *end != ']' && end > line )
            *end-- = 0;

        if( end == line )
            return 1;

        *end = 0;
        *outkey = line+1;
        return 0;
    }

    char* middle = strchr(line, '=');
    if( !middle )
        return 1;

    *middle = 0;

    char* key = line;

    // trim start of key
    while( isspace(*key) && key < middle )
        key++;
    char* keyend = middle-1;
    // trim end of key
    while( isspace(*keyend) && keyend > key )
        *keyend-- = 0;
    *outkey = key;

    char* value = middle+1;

    // remove comments
    char* valueend = value;
    while( valueend < end )
    {
        if( *valueend == ';' || *valueend == '#' )
        {
            *valueend = 0;
            valueend--;
            break;
        }
        valueend++;
    }

    // trim start of value
    while( isspace(*value) && value < end )
        value++;
    // trim end of value
    while( isspace(*valueend) && valueend > value )
        *valueend-- = 0;
    *outvalue = value;

    return 0;
}

int jc_ini_parse_from_file(FILE* f, jc_ini_handler handler, void* ctx)
{
    size_t size = 256;
    char* line = (char*)malloc(size);

    char* section = 0;

    int count = 0;
    while( count++ < 20 && (line = jc_ini_getline(f, &size, line)) )
    {
        char* key = 0;
        char* value = 0;
        if( jc_ini_parseline(line, &key, &value ) )
        {
            fprintf(stderr, "Failed parsing line %d: '%s'\n", count, line);
            free(line);
            return 1;
        }

        if( !key && !value ) // comment
            continue;

        if( key && !value ) // section
        {
            if( section )
                free((void*)section);
            section = strdup(key);
            continue;
        }

        handler(section, key, value, ctx);
    }

    free(line);
    return 0;
}

int jc_ini_parse_from_path(const char* path, jc_ini_handler handler, void* ctx)
{
    FILE* f = fopen(path, "rb");
    if( !f )
        return 1;
    int result = jc_ini_parse_from_file(f, handler, ctx);
    fclose(f);
    return result;
}


#endif // JC_INI_IMPLEMENTATION

