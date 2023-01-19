#include <datrie/datrie.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
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

    while ((read = getline((char **)&line, &len, fp)) != -1 && count < 100000)
    {
        line[read - 1] = '\0';
        datrie_insert(&datrie, line);
        count++;
    }

    datrie_free(&datrie);
}
