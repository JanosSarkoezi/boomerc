#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

const Config default_config = {
    .min_scale = 0.01f,
    .scroll_speed = 1.5f,
    .drag_friction = 6.0f,
    .scale_friction = 4.0f,
};

static char *trim(char *str) {
    char *end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

Config load_config(const char *file_path) {
    Config result = default_config;
    FILE *f = fopen(file_path, "r");
    if (!f) {
        fprintf(stderr, "Could not open config file %s, using defaults\n", file_path);
        return result;
    }

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char *trimmed = trim(line);
        if (trimmed[0] == '\0' || trimmed[0] == '#') continue;

        char *key = strtok(trimmed, "=");
        char *value = strtok(NULL, "=");

        if (key && value) {
            key = trim(key);
            value = trim(value);

            if (strcmp(key, "min_scale") == 0) {
                result.min_scale = atof(value);
            } else if (strcmp(key, "scroll_speed") == 0) {
                result.scroll_speed = atof(value);
            } else if (strcmp(key, "drag_friction") == 0) {
                result.drag_friction = atof(value);
            } else if (strcmp(key, "scale_friction") == 0) {
                result.scale_friction = atof(value);
            } else {
                fprintf(stderr, "Unknown config key `%s`", key);
                exit(1);
            }
        }
    }

    fclose(f);
    return result;
}

void generate_default_config(const char *file_path) {
    FILE *f = fopen(file_path, "w");
    if (!f) {
        perror("fopen");
        exit(1);
    }
    fprintf(f, "min_scale = %f\n", default_config.min_scale);
    fprintf(f, "scroll_speed = %f\n", default_config.scroll_speed);
    fprintf(f, "drag_friction = %f\n", default_config.drag_friction);
    fprintf(f, "scale_friction = %f\n", default_config.scale_friction);
    fclose(f);
}
