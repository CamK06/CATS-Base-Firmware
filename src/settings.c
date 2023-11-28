#include <string.h>
#include "settings.h"
#include "flash.h"

tnc_settings settings;

void settings_load()
{
    flash_read(SETTINGS_FLASH_ADDR, SETTINGS_BUF_SIZE, (char*)&settings);
}

void settings_save()
{
    uint8_t buf[SETTINGS_BUF_SIZE];
    memcpy(buf, 0x00, SETTINGS_BUF_SIZE);
    memcpy(buf, &settings, SETTINGS_BUF_SIZE);

    settings_erase();
    flash_write(SETTINGS_FLASH_ADDR, SETTINGS_BUF_SIZE, (char*)buf);
}

void settings_erase()
{
    flash_erase(SETTINGS_FLASH_ADDR, SETTINGS_BUF_SIZE);
}