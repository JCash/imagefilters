#pragma once

enum jc_argv_type
{
    JC_ARGV_TYPE_STRING,
    JC_ARGV_TYPE_INT,
    JC_ARGV_TYPE_FLOAT,
};

struct jc_argv_value
{
    jc_argv_type type;
    union {
        const char* s;
        int i;
        float f;
    };
};

struct jc_argv_argument
{
    const char* name;
    jc_argv_value value;

    jc_argv_argument* next;
};





int jc_argv_parse()