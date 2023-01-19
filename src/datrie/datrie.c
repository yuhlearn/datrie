#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <datrie/datrie.h>

#define INITIAL_CAPACITY 1024

#define START_STATE 1
#define FIRST_LINK 0
#define LEAF_STATE -1
#define INIT_LAST_LINK 2

#define MIN(a, b) \
    (((a) < (b)) ? (a) : (b))

#define MAX(a, b) \
    (((a) > (b)) ? (a) : (b))

#define DATRIE_GET_LAST_LINK(datrie) \
    (-datrie->base[FIRST_LINK])

#define DATRIE_GET_SECOND_LINK(datrie) \
    (-datrie->check[FIRST_LINK])

#define DATRIE_GET_NEXT_LINK(datrie, link) \
    (-datrie->check[link])

#define DATRIE_GET_PREVIOUS_LINK(datrie, link) \
    (-datrie->base[link])

#define DATRIE_GET_NEXT_STATE(datrie, state, str, index) \
    (datrie->base[state] + str[index])

static void datrie_ensure_capacity(DATrie *datrie,
                                   const int state);

int id = 0;

void datrie_init(DATrie *datrie)
{
    datrie->capacity = INITIAL_CAPACITY;
    datrie->min_capacity = INITIAL_CAPACITY;
    datrie->max_state = START_STATE;

    datrie->base = calloc(INITIAL_CAPACITY, sizeof(int));
    datrie->check = calloc(INITIAL_CAPACITY, sizeof(int));
    datrie->tail = calloc(INITIAL_CAPACITY, sizeof(uint8_t *));

    datrie->base[START_STATE] = INIT_LAST_LINK;
    datrie->base[FIRST_LINK] = -INIT_LAST_LINK;
    datrie->base[INIT_LAST_LINK] = -FIRST_LINK;

    datrie->check[FIRST_LINK] = -INIT_LAST_LINK;
    datrie->check[INIT_LAST_LINK] = -FIRST_LINK;
}

void datrie_free(DATrie *datrie)
{
    for (int i = 0; i < datrie->max_state; i++)
    {
        if (datrie->tail[i] != NULL)
        {
            free(datrie->tail[i]);
            datrie->tail[i] = NULL;
        }
    }

    free(datrie->base);
    free(datrie->check);
    free(datrie->tail);
}

int datrie_find(DATrie *datrie,
                const uint8_t *str)
{
    const int str_length = strlen(str);
    const int last_link = DATRIE_GET_LAST_LINK(datrie);

    int state = START_STATE;
    int index = 0;

    while ((index < str_length) && (datrie->base[state] != LEAF_STATE))
    {
        int next_state = DATRIE_GET_NEXT_STATE(datrie, state, str, index);

        if ((next_state >= last_link) || (datrie->check[next_state] != state))
            return 0;

        index++;
        state = next_state;
    }

    uint8_t *suffix = datrie->tail[state];

    if ((suffix != NULL) && (strcmp(suffix, &str[index]) == 0))
        return 1;

    return 0;
}

static int datrie_get_link_after(DATrie *datrie,
                                 const int state)
{
    int index = state;

    while (datrie->check[index] > FIRST_LINK)
        index++;

    return index;
}

static int datrie_get_link_after_link(DATrie *datrie,
                                      const int initial_link,
                                      const int state)
{
    int next_link = initial_link;

    datrie_ensure_capacity(datrie, state + 1);

    while (next_link <= state)
        next_link = DATRIE_GET_NEXT_LINK(datrie, next_link);

    return next_link;
}

static int datrie_find_base(DATrie *datrie,
                            const int character)
{
    int next_link =
        datrie_get_link_after_link(datrie, FIRST_LINK, INIT_LAST_LINK + character);

    return next_link - character;
}

static int datrie_find_common_base(DATrie *datrie,
                                   const uint8_t character_a,
                                   const uint8_t character_b)
{
    const int min = (int)MIN(character_a, character_b);
    const int max = (int)MAX(character_a, character_b);

    int next_link = datrie_get_link_after_link(datrie, FIRST_LINK, INIT_LAST_LINK + min);

    while (next_link > 0)
    {
        int new_base = next_link - min;

        datrie_ensure_capacity(datrie, new_base + max);

        if ((datrie->check[new_base + min] < 0) && (datrie->check[new_base + max] < 0))
            return new_base;

        next_link = DATRIE_GET_NEXT_LINK(datrie, next_link);
    }

    printf("Error: Failed to traverse the G-list.\n");
    exit(EXIT_FAILURE);
}

static void datrie_extend_glink(DATrie *datrie,
                                const int new_last_link)
{
    int link = DATRIE_GET_LAST_LINK(datrie);

    if (new_last_link > link)
    {
        // base[link] already points to the previous link,
        // make check[link] point to the next link
        datrie->check[link] = -(link + 1);

        // Iterate over the links between the previously last link and the new one
        for (int next_link = link + 1; next_link < new_last_link; next_link++)
        {
            datrie->check[next_link] = -(next_link + 1);
            datrie->base[next_link] = -(next_link - 1);
        }

        // Set the new last link
        datrie->check[new_last_link] = -FIRST_LINK;
        datrie->base[new_last_link] = -(new_last_link - 1);
        datrie->base[FIRST_LINK] = -new_last_link;
    }
}

static void datrie_ensure_capacity(DATrie *datrie,
                                   const int state)
{
    if (state + 2 > datrie->capacity)
    {
        const int old_capacity = datrie->capacity;
        const int new_capacity = old_capacity * 2;

        datrie->base = realloc(datrie->base, sizeof(int) * new_capacity);
        datrie->check = realloc(datrie->check, sizeof(int) * new_capacity);
        datrie->tail = realloc(datrie->tail, sizeof(uint8_t *) * new_capacity);

        if (datrie->base == NULL || datrie->check == NULL || datrie->tail == NULL)
        {
            printf("Error: Failed to allocate enough memory to grow trie.\n");
            exit(EXIT_FAILURE);
        }

        memset(&datrie->base[old_capacity], 0, sizeof(int) * old_capacity);
        memset(&datrie->check[old_capacity], 0, sizeof(int) * old_capacity);
        memset(&datrie->tail[old_capacity], 0, sizeof(uint8_t *) * old_capacity);

        datrie->capacity = new_capacity;
    }

    datrie_extend_glink(datrie, state + 1);
}

static void datrie_break_link(DATrie *datrie,
                              const int link)
{
    int next_link = DATRIE_GET_NEXT_LINK(datrie, link);
    int previous_link = DATRIE_GET_PREVIOUS_LINK(datrie, link);

    datrie->check[previous_link] = -next_link;
    datrie->base[next_link] = -previous_link;

    if (link > datrie->max_state)
        datrie->max_state = link;
}

static int datrie_insert_prefix(DATrie *datrie,
                                const uint8_t *prefix,
                                const int src_state)
{
    int state = src_state;

    for (int i = 0; prefix[i] != '\0'; i++)
    {
        int character = (int)prefix[i];
        int next_base = datrie_find_base(datrie, character);
        int next_state = next_base + character;

        datrie_ensure_capacity(datrie, next_state);
        datrie_break_link(datrie, next_state);

        datrie->base[state] = next_base;
        datrie->check[next_state] = state;

        state = next_state;
    }

    return state;
}

static int datrie_count_transitions(DATrie *datrie,
                                    const int state,
                                    const int character)
{
    const int state_base = datrie->base[state];
    const int last_link = DATRIE_GET_LAST_LINK(datrie);

    // Is one reserved for character
    int count = (character == -1) ? 0 : 1;

    // Leaves don't have transitions
    if (state_base == LEAF_STATE)
        return 0;

    for (int c = 0; (c < 256) && (state_base + c < last_link); c++)
        if (datrie->check[state_base + c] == state)
            count++;

    return count;
}

static void datrie_fill_transitions(DATrie *datrie,
                                    int *dest,
                                    const int count,
                                    const int state,
                                    const int character)
{
    const int state_base = datrie->base[state];

    for (int c = 0, i = 0; i < count; c++)
    {
        // printf("c %d \n", character);
        if ((datrie->check[state_base + c] == state) || (c == character))
            dest[i++] = c;
    }
}

static void datrie_copy_state(DATrie *datrie,
                              const int from_state,
                              const int to_state)
{
    datrie->check[to_state] = datrie->check[from_state];
    datrie->base[to_state] = datrie->base[from_state];
    datrie->tail[to_state] = datrie->tail[from_state];
}

static void datrie_make_link(DATrie *datrie,
                             const int state,
                             const int next_link)
{
    int previous_link = DATRIE_GET_PREVIOUS_LINK(datrie, next_link);

    datrie->check[state] = -next_link;
    datrie->check[previous_link] = -state;

    datrie->base[state] = -previous_link;
    datrie->base[next_link] = -state;

    datrie->tail[state] = NULL;
}

static int datrie_find_base_array(DATrie *datrie,
                                  const int *characters,
                                  const int set_count)
{
    const int max = characters[set_count - 1];
    const int first_character = characters[0];

    int next_link = datrie_get_link_after_link(datrie, FIRST_LINK, INIT_LAST_LINK + first_character);

    while (next_link > 0)
    {
        int count = 1;
        int new_base = next_link - first_character;

        datrie_ensure_capacity(datrie, new_base + max);

        while ((count < set_count) && (datrie->check[new_base + characters[count]] < 0))
            count++;

        if (count == set_count)
            return new_base;

        next_link = DATRIE_GET_NEXT_LINK(datrie, next_link);
    }

    exit(EXIT_FAILURE);
}

static void datrie_move_base(DATrie *datrie,
                             const int state,
                             const int *transitions,
                             const int count,
                             const int character)
{
    const int old_base = datrie->base[state];
    const int new_base = datrie_find_base_array(datrie, transitions, count);
    int next_link = DATRIE_GET_SECOND_LINK(datrie);

    datrie->base[state] = new_base;

    for (int i = 0; i < count; i++)
    {
        int c = transitions[i];

        if (c != character)
        {
            int old_state = old_base + c;
            int new_state = new_base + c;

            int cnt = datrie_count_transitions(datrie, old_state, -1);
            int trans[cnt];
            datrie_fill_transitions(datrie, trans, cnt, old_state, -1);

            for (int j = 0; j < cnt; j++)
                datrie->check[datrie->base[old_state] + trans[j]] = new_state;

            if (next_link == new_state)
                next_link = DATRIE_GET_NEXT_LINK(datrie, next_link);

            datrie_break_link(datrie, new_state);
            datrie_copy_state(datrie, old_state, new_state);
            next_link = datrie_get_link_after_link(datrie, next_link, old_state);
            datrie_make_link(datrie, old_state, next_link);
        }
    }
}

static void datrie_insert_new_leaf(DATrie *datrie,
                                   const int state,
                                   const uint8_t *str,
                                   const int index)
{
    int next_state = DATRIE_GET_NEXT_STATE(datrie, state, str, index);

    datrie_ensure_capacity(datrie, next_state);

    datrie_break_link(datrie, next_state);

    datrie->check[next_state] = state;
    datrie->base[next_state] = LEAF_STATE;
    datrie->tail[next_state] = strdup(&str[index + 1]);
}

static void datrie_insert_new_branch(DATrie *datrie,
                                     const uint8_t *str,
                                     const int index,
                                     const int state)
{
    int next_state = DATRIE_GET_NEXT_STATE(datrie, state, str, index);

    datrie_ensure_capacity(datrie, next_state);

    if (datrie->check[next_state] >= START_STATE)
    {
        // Move the base of state OR the base of the parent of the obstructing state
        const int current_parent = datrie->check[next_state];
        int character = (int)str[index];

        int count = datrie_count_transitions(datrie, state, character);
        int transitions_a[count];
        int *transitions = transitions_a;

        datrie_fill_transitions(datrie, transitions, count, state, character);

        if (datrie->check[state] != current_parent)
        {
            const int count_b = datrie_count_transitions(datrie, state, character);

            if (count > count_b)
            {
                character = -1;
                count = count_b;
                int transitions_b[count];
                transitions = transitions_b;

                datrie_fill_transitions(datrie, transitions, count, state, character);
            }
        }

        datrie_move_base(datrie, state, transitions, count, character);
    }

    datrie_insert_new_leaf(datrie, state, str, index);
}

static void datrie_insert_unfold_tail(DATrie *datrie,
                                      const uint8_t *prefix,
                                      const uint8_t *suffix_a,
                                      const uint8_t *suffix_b,
                                      const int src_state)
{
    int state = datrie_insert_prefix(datrie, prefix, src_state);

    if (suffix_a[0] == '\0')
    {
        datrie->base[state] = datrie_find_base(datrie, suffix_b[0]);
        datrie->tail[state] = strdup("");

        datrie_insert_new_leaf(datrie, state, suffix_b, 0);
    }
    else if (suffix_b[0] == '\0')
    {
        datrie->base[state] = datrie_find_base(datrie, suffix_a[0]);
        datrie->tail[state] = strdup("");

        datrie_insert_new_leaf(datrie, state, suffix_a, 0);
    }
    else
    {
        datrie->base[state] =
            datrie_find_common_base(datrie, suffix_a[0], suffix_b[0]);

        datrie_insert_new_leaf(datrie, state, suffix_a, 0);
        datrie_insert_new_leaf(datrie, state, suffix_b, 0);
    }
}

void datrie_insert(DATrie *datrie,
                   const uint8_t *str)
{
    const int str_length = strlen(str);
    const int last_link = DATRIE_GET_LAST_LINK(datrie);

    int state = START_STATE;
    int index = 0;

    id++;

    while (index < str_length)
    {
        int next_state = DATRIE_GET_NEXT_STATE(datrie, state, str, index);

        // We reached unknown territory OR the next state is taken
        if ((next_state >= last_link) || (datrie->check[next_state] != state))
        {
            // Insert a new branch starting from the previous state
            datrie_insert_new_branch(datrie, str, index, state);
            return;
        }

        // Increment the index early so it points to the next character,
        // because this is what will be stored in tail
        index++;

        // We reached a leaf state
        if (datrie->base[next_state] == LEAF_STATE)
        {
            uint8_t *suffix_a = (uint8_t *)&str[index];
            uint8_t *suffix_b = datrie->tail[next_state];
            int count = 0;

            // The length of the common prefix
            while (suffix_a[count] == suffix_b[count] &&
                   suffix_a[count] != '\0')
                count++;

            if (suffix_a[count] != '\0')
            {
                uint8_t prefix[count + 1];
                memcpy(prefix, suffix_a, count);
                prefix[count] = '\0';

                datrie_insert_unfold_tail(datrie,
                                          prefix,
                                          &suffix_a[count],
                                          &suffix_b[count],
                                          next_state);
            }

            // The string already existed in the trie
            return;
        }

        state = next_state;
    }

    // We exhaused the string and state is not a leaf,
    // so we just insert where we are
    if (datrie->tail[state] == NULL)
        datrie->tail[state] = strdup("");
}