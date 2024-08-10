#define _GNU_SOURCE
#include "arch.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

int arch_init(void) { return 0; }

int arch_load(char *name, arch_load_out_cb cb, void *context) {
    FILE *fp = fopen(name, "r");
    if (!fp) {
        return 1;
    }
    char line[256];
    while (fgets(line, 256, fp) != NULL) {
        cb(line, context);
    }
    fclose(fp);
    return 0;
}

int arch_save(char *name, arch_save_cb cb, void *context) { return 1; }

int arch_dir(arch_dir_out_cb cb, void *context) { return 1; }

int arch_delete(char *name) { return 1; }
