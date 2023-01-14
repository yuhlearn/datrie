#ifndef _DATRIE_H
#define _DATRIE_H

#include <stdint.h>

typedef struct
{
    int *base;
    int *check;
    uint8_t **tail;

    int capacity;
    int min_capacity;
    int max_state;
} DATrie;

void datrie_init(DATrie *datrie);

void datrie_free(DATrie *datrie);

int datrie_find(DATrie *datrie, const uint8_t *str);

void datrie_insert(DATrie *datrie, const uint8_t *str);

#endif