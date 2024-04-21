#include "settings.h"
#include "drivers/flash.h"
#include "drivers/serial.h"

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
        "BEACON\0",
        { 0 },
        CATS_BOOL
    },
    {
        "BEACON_INTERVAL\0",
        { 5 & 0xFF, 5 >> 8 },
        CATS_UINT16
    },
    {
        "STATUS\0",
        "Hello CATS world!\0",
        CATS_STRING
    },
    {
        "DIGIPEAT\0",
        { 0 },
        CATS_BOOL
    },
    {
        "SSID\0",
        { 0 },
        CATS_UINT8
    }
};
const int var_count = sizeof(env_vars) / sizeof(cats_env_var_t);

cats_env_var_t* get_var(const char* name)
{
    for(int i = 0; i < var_count; i++) {
        if(strcmp(env_vars[i].name, name) == 0) {
            return &env_vars[i];
        }
    }
    return NULL;
}

cats_env_var_t** get_all_vars()
{
    cats_env_var_t** vars = malloc(sizeof(cats_env_var_t) * var_count);
    for(int i = 0; i < var_count; i++) {
        vars[i] = &env_vars[i];
    }
    return vars;
}

char* var_type_to_str(const cats_env_var_t* var)
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

int str_to_var(cats_env_var_t* var, const char* str)
{
    switch(var->type) {
        case CATS_STRING:
        if(strlen(str) <= 32) {
            strcpy(var->val, str);
            var->val[strlen(str)] = '\0';
        }
        else {
            return -1;
        }
        break;

        case CATS_BOOL:
        if(strcmp(str, "TRUE") == 0) {
            var->val[0] = 1;
        }
        else if(strcmp(str, "FALSE") == 0) {
            var->val[0] = 0;
        }
        else {
            return -1;
        }
        break;

        case CATS_UINT8:
        uint8_t val8 = atoi(str);
        var->val[0] = val8;
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

char* var_to_str(const cats_env_var_t* var)
{
    char* out = malloc(32);
    switch(var->type) {
        case CATS_STRING:
        strcpy(out, var->val);
        break;

        case CATS_BOOL:
        sprintf(out, var->val[0] ? "TRUE" : "FALSE");
        break;

        case CATS_UINT8:
        sprintf(out, "%d", var->val[0]);
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

const int var_val_int(const cats_env_var_t* var) // TEMPORARY! TODO: Remove this when new settings system is done!
{
    int ret;
    memcpy(&ret, var->val, sizeof(uint32_t));
    return ret;
}

uint16_t crc16(uint8_t* data, int len)
{
	uint16_t crc = 0xFFFF;
	for(int i = 0; i < len; i++) {
		crc ^= data[i];
		for(uint8_t j = 0; j < 8; j++) {
			crc = (crc & 1) != 0 ? (crc >> 1) ^ 0x8408 : crc >> 1;
        }
	}
	return ~crc;
}

void settings_load()
{
    char buf[sizeof(env_vars)+ (2 * sizeof(uint16_t))];
    uint16_t len, crc;
   
    flash_read(sizeof(uint16_t), (char*)&len);
    flash_read(len + sizeof(uint16_t), buf);
    memcpy(&crc, buf + len, sizeof(uint16_t));

    if(crc != crc16(buf + sizeof(uint16_t), len - sizeof(uint16_t))) {
        printf("%x != %x\n", crc, crc16(buf + sizeof(uint16_t), len - sizeof(uint16_t)));
        serial_write_str("Settings CRC checksum is invalid!\n");
        serial_write_str("Settings will be reset!\n");
        return;
    }

    int var = 0;
    for(int i = sizeof(uint16_t); i - sizeof(uint16_t) < len; i++) {
        var = buf[i];
        if(var >= var_count) {
            serial_write_str("Tried to read non-existent variable!\n");
            return;
        }
        memset(env_vars[var].val, 0x00, 255);
        memcpy(env_vars[var].val, &buf[i+2], buf[i+1]);
        i += buf[i + 1] + 1;
        if(len-i <= 3) {
            break;
        }
    }
}

void settings_save()
{
    char buf[sizeof(env_vars)];
    uint16_t idx = sizeof(uint16_t);
    for(int i = 0; i < var_count; i++) {
        buf[idx++] = (uint8_t)i;    // Variable index
        switch(env_vars[i].type) {  // Variable length
            case CATS_BOOL:
            case CATS_UINT8:
                buf[idx++] = 1;
            break;
            case CATS_UINT16:
                buf[idx++] = 2;
            break;
            case CATS_UINT32:
                buf[idx++] = 4;
            break;
            case CATS_STRING:
                buf[idx++] = strlen(env_vars[i].val) + 1; // Shouldn't exceed 255, CHANGE THIS IF VALUES ARE ALLOWED >255
            break;
        }
        memcpy(&buf[idx], env_vars[i].val, buf[idx - 1]); // Variable data
        idx += buf[idx - 1];
    }
    uint16_t crc = crc16(buf+sizeof(uint16_t), idx - sizeof(uint16_t));
    memcpy(buf, &idx, sizeof(uint16_t));    // Memory size
    memcpy(buf+idx, &crc, sizeof(uint16_t));    // CRC
    idx += sizeof(uint16_t);

    flash_erase(idx);
    flash_write(idx, buf);
}