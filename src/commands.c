#include "shell.h"
#include "settings.h"
#include "commands.h"
#include "version.h"
#include "config.h"
#include "radio.h"

#include "drivers/mcu.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int cmd_tx(int argc, char* argv[])
{
    if(argc <= 1) {
        return SHELL_FAIL;
    }

    char val[255];
    strcpy(val+2, argv[1]);
    int idx = strlen(argv[1])+2;
    for(int i = 2; i < argc; i++) {
        val[idx++] = ' ';
        strcpy(val+idx, argv[i]);
        idx += strlen(argv[i]);
    }
    const uint16_t len = strlen(val+2);
    memcpy(val, &len, sizeof(uint16_t));

    radio_send(val, len+2);
    printf("Sent %d bytes: %s\n", strlen(val+2), val+2);
    return SHELL_OK;
}

shell_cmd_t commands[] = {
    {
        "help",
        "List available commands or get usage info for a specific command",
        "[command]",
        0,
        &cmd_help
    },
    {
        "ver",
        "Display the firmware version and build strings",
        "",
        0,
        &cmd_ver
    },
    {
        "get",
        "Get the value of a setting",
        "[setting]",
        1,
        &cmd_get
    },
    {
        "set",
        "Set the value of a setting",
        "[setting] [value]",
        2,
        &cmd_set
    },
    {
        "list",
        "List all settings and their values",
        "",
        0,
        &cmd_list
    },
    {
        "store",
        "Store all settings to non-volatile memory",
        "",
        0,
        &cmd_store_settings
    },
    {
        "reset",
        "Software reboot",
        "flash",
        0,
        &cmd_reboot
    },
    {
        "tx",
        "Software reboot",
        "flash",
        0,
        &cmd_tx
    }
};
int cmdCount = sizeof(commands)/sizeof(shell_cmd_t);

int cmd_store_settings(int argc, char* argv[])
{
    // TODO: Actually save
    settings_save();
    puts("Settings saved to memory!");
    return SHELL_OK;
}

int cmd_list(int argc, char* argv[])
{
    cats_env_var_t** vars = get_all_vars();
    for(int i = 0; i < varCount; i++) {
        char* str = var_to_str(vars[i]);
        printf("%s=%s\n", vars[i]->name, str);
        free(str);
    }
    free(vars);

    return SHELL_OK;
}

int cmd_get(int argc, char* argv[])
{
    cats_env_var_t* v = get_var(argv[1]);
    if(v != NULL) {
        char* str = var_to_str(v);
        printf("%s=%s\n", v->name, str);
        free(str);
    }
    else {
        printf("Variable '%s' not found!\n", argv[1]);
        return SHELL_FAIL;
    }

    return SHELL_OK;
}

int cmd_set(int argc, char* argv[])
{
    char val[255];
    strcpy(val, argv[2]);
    cats_env_var_t* v = get_var(argv[1]);
    if(v == NULL) {
        printf("Variable '%s' not found!\n", argv[1]);
        return SHELL_FAIL;
    }
    if(v->type == CATS_STRING) { // Handle spaces
        int idx = strlen(argv[2]);
        for(int i = 3; i < argc; i++) {
            val[idx++] = ' ';
            strcpy(val+idx, argv[i]);
            idx += strlen(argv[i]);
        }
    }
    if(strlen(val) > 255) { // TODO: Make this larger, or just find a less dumb solution PLEASE
        printf("Cannot set variable to a value greater than 255 bytes!");
        return SHELL_FAIL;
    }
    if(str_to_var(v, val) != 0) {
        printf("Failed to set variable %s to '%s'\n", argv[1], argv[2]);
        printf("Variable expects type %s\n", var_type_to_str(v));
        return SHELL_FAIL;
    }
    printf("%s=%s\n", v->name, var_to_str(v));
    
    return SHELL_OK;
}

int cmd_help(int argc, char* argv[])
{
    for(int i = 0; i < cmdCount; i++) {
        if(argc == 1)
            printf("%s - %s\n", commands[i].cmd, commands[i].desc);
        if(strcmp(commands[i].cmd, argv[1]) == 0) {
            printf("%s - %s\n", commands[i].cmd, commands[i].desc);
            printf("Usage: %s %s\n", commands[i].cmd, commands[i].usage);
        }
    }
            
    return SHELL_OK;
}

int cmd_ver(int argc, char* argv[])
{
    puts(DEVICE_NAME);
    puts("Firmware Version: " CATS_FW_VERSION);
    puts("Build: " BUILD_STR);

    return SHELL_OK;
}

int cmd_reboot(int argc, char* argv[])
{
    int r;
    if(argc == 2 && (strcmp(argv[1], "flash") == 0))
        r = mcu_flash();
    else
        r = mcu_reset();
    if(r < 0) {
        puts("Not supported!");
        return SHELL_OK;
    }
    else {
        puts("Rebooting...");
        while(1);
    }
}