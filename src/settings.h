#ifndef CATS_FW_SETTINGS_H
#define CATS_FW_SETTINGS_H

// Possible data types for environment variables
typedef enum cats_env_var_type {
    CATS_STRING,
    CATS_BOOL,
    CATS_UINT8,
    CATS_UINT16,
    CATS_UINT32
}   cats_env_var_type_t;

typedef struct cats_env_var {
    char name[16];
    char val[255];
    cats_env_var_type_t type;
} cats_env_var_t;

extern cats_env_var_t env_vars[];

int str_to_var(cats_env_var_t* var, const char* str);
char* var_type_to_str(const cats_env_var_t* var);
char* var_to_str(const cats_env_var_t* var);
cats_env_var_t* get_var(const char* name);
cats_env_var_t** get_all_vars();
const int var_val_int(const cats_env_var_t* var);

// Load settings from flash
void settings_load();
// Save settings to flash
void settings_save();

#endif // CATS_FW_SETTINGS_H