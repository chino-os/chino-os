#define _GNU_SOURCE
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dictionary.h>

typedef struct entry entry;
struct entry {
    entry *next;
    char *name;
    void *value;
};

#if ARCH != ARCH_XMEGA
#define HASHSIZE 13
#else
#define HASHSIZE 101
#endif

struct dictionary {
    entry *hashtab[HASHSIZE];
};

static unsigned int hash(char *name) {
    unsigned int hashval;
    for (hashval = 0; *name != '\0'; name++) {
        hashval = *name + 31 * hashval;
    }
    return hashval % HASHSIZE;
}

static entry *_get(dictionary *d, char *name) {
    // printf("get %p %s\n", d, name);
    entry *entry;
    for (entry = d->hashtab[hash(name)]; entry != NULL; entry = entry->next) {
        if (strcmp(name, entry->name) == 0) {
            // printf("\tgot %p\n", entry);
            return entry;
        }
    }
    // puts("not\n");
    return NULL;
}

void *dictionary_get(dictionary *d, char *name) {
    entry *entry = _get(d, name);

    if (entry) {
        return entry->value;
    }

    return NULL;
}

bool dictionary_has(dictionary *d, char *name) {
    entry *entry = _get(d, name);

    if (entry) {
        return true;
    }

    return false;
}

void dictionary_put(dictionary *d, char *name, void *value) {
    // printf("put %p %s\n", d, name);
    entry *element;
    unsigned int hashval;

    element = _get(d, name);

    if (element == NULL) {
        element = (entry *)malloc(sizeof(*element));
        if (element == NULL || (element->name = strdup(name)) == NULL) {
            return;
        }
        hashval = hash(name);
        element->next = d->hashtab[hashval];
        d->hashtab[hashval] = element;
    }
    // printf("entry %p\n", element);
    element->value = value;
}

void *dictionary_del(dictionary *d, char *name) {
    entry *root = d->hashtab[hash(name)];

    if (root == NULL) {
        return NULL;
    }

    if (strcmp(name, root->name) == 0) {
        d->hashtab[hash(name)] = root->next;
        void *value = root->value;
        free(root->name);
        free(root);
        return value;
    }

    entry *element = root;
    while (element->next) {
        entry *next = element->next;
        if (strcmp(name, next->name) == 0) {
            element->next = next->next;
            void *value = next->value;
            free(next->name);
            free(next);
            return value;
        }
    }

    return NULL;
}

void dictionary_each(dictionary *d, dictionary_each_cb cb, void *context) {
    entry *next_entry = NULL;

    if (!cb) {
        return;
    }

    for (size_t i = 0; i < HASHSIZE; i++) {
        entry *entry = d->hashtab[i];
        while (entry) {
            next_entry = entry->next;
            cb(entry->name, entry->value, context);
            entry = next_entry;
        }
    }
}

dictionary *dictionary_new() {
    // printf("dictionary size: %ld\n", sizeof(dictionary));
    dictionary *d = malloc(sizeof(dictionary));
    if (d == NULL) {
        return NULL;
    }
    for (size_t i = 0; i < HASHSIZE; i++) {
        d->hashtab[i] = NULL;
    }
    return d;
}

typedef struct {
    dictionary *d;
    dictionary_each_cb cb;
} _free_s;

static void destroy_cb_pass_1(char *name, void *value, void *context) {
    _free_s *ctx = (_free_s *)context;
    // dictionary *d = ctx->d;
    dictionary_each_cb free_cb = ctx->cb;
    free_cb(name, value, NULL);
    // dictionary_del(d, name);
}

static void destroy_cb_pass_2(char *name, void *value, void *context) {
    _free_s *ctx = (_free_s *)context;
    dictionary *d = ctx->d;
    // dictionary_each_cb free_cb = ctx->cb;
    // free_cb(name, value, NULL);
    dictionary_del(d, name);
}

void dictionary_destroy(dictionary *d, dictionary_each_cb free_cb) {
    _free_s ctx = {.d = d, .cb = free_cb};
    dictionary_each(d, destroy_cb_pass_1, &ctx);
    dictionary_each(d, destroy_cb_pass_2, &ctx);
    free(d);
}
