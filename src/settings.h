#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "flash.h"

typedef struct tnc_settings
{
    int firmware_version;
    char callsign[16];
    bool transmit;
    bool shell_echo;
} tnc_settings;

#define SETTINGS_FLASH_ADDR (flash_size() - (flash_sector_size()*sizeof(tnc_settings)))
#define SETTINGS_BUF_SIZE flash_page_size()*(sizeof(tnc_settings)/flash_page_size())

void settings_load();
void settings_save();
void settings_erase();