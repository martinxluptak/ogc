#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include "gc.h"

#define IS_DEL(c, d) (c == d)
#define THREADS_CNT 4

static inline char *my_strdup(char *s)
{
    size_t size = strlen(s);
    char *dup = gc_alloc(sizeof(char) * (size + 1));
    strcpy(dup, s);
    return dup;
}

static int count_words(char *s, char delimiter)
{
    int i = -1, words = 0;
    while (s[++i])
        if (((i != 0 && IS_DEL(s[i], delimiter) &&
              !IS_DEL(s[i - 1], delimiter)) ||
             (s[i + 1] == '\0' && !IS_DEL(s[i], delimiter))))
            words++;
    return words;
}

static char *get_word(char *s, char delimiter)
{
    int i = -1, size = 0;
    while (s[++i] && !IS_DEL(s[i], delimiter))
        size++;
    char *word = gc_alloc(sizeof(char) * (size + 1));
    word[size] = '\0';
    while (*s && !IS_DEL(*s, delimiter)) {
        *word++ = *s;
        s++;
    }
    return word - size;
}

char **my_strsep(char *s, char delimiter)
{
    if (!s)
        return NULL;

    char **tab;
    int size = count_words(s, delimiter);
    if (!(tab = gc_alloc((size + 1) * sizeof(char *))))
        return NULL;

    tab[size] = 0;
    char *str = s;
    while (*str) {
        if (!IS_DEL(*str, delimiter)) {
            *tab++ = get_word(str, delimiter);
            str += strlen(get_word(str, delimiter)) - 1;
        }
        str++;
    }
    return tab - size;
}

void *strsep_task(void *argc)
{
    gc_init(&argc, 10);

    char **sep =
        my_strsep("hello,world,world2,hello2,world3,hello3,hello4", ',');
    assert(sep);

    char str1[] = "1234567890";
    char *str2 = my_strdup(str1);

    printf("%s, %s\n", str1, str2);

    printf("%s\n", sep[0]);
    printf("%s\n", sep[1]);

    gc_dump_internals();
    gc_destroy();
    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t threads[THREADS_CNT];
    for (int i = 0; i < THREADS_CNT; ++i)
        pthread_create(&threads[i], NULL, strsep_task, NULL);

    for (int i = 0; i < THREADS_CNT; ++i)
        pthread_join(threads[i], NULL);

    strsep_task(&argc);
    return 0;
}
