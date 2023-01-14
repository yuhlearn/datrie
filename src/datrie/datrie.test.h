#ifndef _DATRIE_TEST_H
#define _DATRIE_TEST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <CUnit/Basic.h>
#include <datrie/datrie.h>

void datrie_test()
{
    FILE *fp;
    uint8_t *line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen("./data/strings.txt", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    DATrie datrie;
    datrie_init(&datrie);

    int count = 0;

    while ((read = getline(&line, &len, fp)) != -1 && count < 100000)
    {
        line[read - 1] = '\0';
        datrie_insert(&datrie, line);
        int found = datrie_find(&datrie, line);
        CU_ASSERT_TRUE(found);
        if (!found)
            printf("%s\n", line);
        count++;
    }

    datrie_free(&datrie);
}

#endif