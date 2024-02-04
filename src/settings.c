#include "settings.h"
#include "flash.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

cats_env_var_t env_vars[] = {
    { 
        "CALLSIGN\0",
        "N0CALL\0",
        CATS_STRING
    },
    {
        "TX\0",
        { 0 },
        CATS_BOOL
    },
    {
        "INT16\0",
        { 0 },
        CATS_UINT16
    },
    {
        "INT32\0",
        { 0 },
        CATS_UINT32
    }
};
int varCount = sizeof(env_vars)/sizeof(cats_env_var_t);

cats_env_var_t* get_var(char* name)
{
    for(int i = 0; i < varCount; i++)
        if(strcmp(env_vars[i].name, name) == 0)
            return &env_vars[i];
    return NULL;
}

cats_env_var_t** get_all_vars()
{
    cats_env_var_t** vars = malloc(sizeof(cats_env_var_t)*varCount);
    for(int i = 0; i < varCount; i++)
        vars[i] = &env_vars[i];
    return vars;
}

char* var_type_to_str(cats_env_var_t* var)
{
    char* out = malloc(16);
    switch(var->type) {
        case CATS_STRING:
        sprintf(out, "STRING");
        break;
        case CATS_BOOL:
        sprintf(out, "BOOL");
        break;
        case CATS_UINT16:
        sprintf(out, "UINT16");
        break;
        case CATS_UINT32:
        sprintf(out, "UINT32");
        break;
    }
    return out;
}

int str_to_var(cats_env_var_t* var, char* str)
{
    switch(var->type) {
        case CATS_STRING:
        if(strlen(str) <= 32)
            strcpy(var->val, str);
        else
            return -1;
        break;

        case CATS_BOOL:
        if(strcmp(str, "TRUE") == 0)
            var->val[0] = 1;
        else if(strcmp(str, "FALSE") == 0)
            var->val[0] = 0;
        else 
            return -1;
        break;

        case CATS_UINT16:
        uint16_t val16 = atoi(str);
        memcpy(var->val, &val16, sizeof(uint16_t));
        break;

        case CATS_UINT32:
        uint32_t val32 = atoi(str);
        memcpy(var->val, &val32, sizeof(uint32_t));
        break;
    }
    return 0;
}

char* var_to_str(cats_env_var_t* var)
{
    char* out = malloc(32);
    switch(var->type) {
        case CATS_STRING:
        strcpy(out, var->val);
        break;

        case CATS_BOOL:
        sprintf(out, var->val[0] ? "TRUE" : "FALSE");
        break;

        case CATS_UINT16:
        uint16_t val16;
        memcpy(&val16, var->val, sizeof(uint16_t));
        sprintf(out, "%d", val16);
        break;

        case CATS_UINT32:
        uint32_t val32;
        memcpy(&val32, var->val, sizeof(uint32_t));
        sprintf(out, "%d", val32);
        break;
    }
    return out;
}

void settings_load()
{
    flash_read(SETTINGS_FLASH_ADDR, SETTINGS_BUF_SIZE, (char*)&env_vars);
}

void settings_save()
{
    uint8_t buf[SETTINGS_BUF_SIZE];
    memcpy(buf, 0x00, SETTINGS_BUF_SIZE);
    memcpy(buf, &env_vars, SETTINGS_BUF_SIZE);

    settings_erase();
    flash_write(SETTINGS_FLASH_ADDR, SETTINGS_BUF_SIZE, (char*)buf);
}

void settings_erase()
{
    flash_erase(SETTINGS_FLASH_ADDR, SETTINGS_BUF_SIZE);
}