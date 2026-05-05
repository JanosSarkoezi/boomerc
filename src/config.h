#ifndef CONFIG_H_
#define CONFIG_H_

typedef struct {
    float min_scale;
    float scroll_speed;
    float drag_friction;
    float scale_friction;
} Config;

extern const Config default_config;

Config load_config(const char *file_path);
void generate_default_config(const char *file_path);

#endif // CONFIG_H_
