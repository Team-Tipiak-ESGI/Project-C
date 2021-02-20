#include "Item.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Insert a new element in the list
 * @param start
 * @param key
 * @param value
 */
void Item__insert(Item *start, char * key, char * value) {
    // Create new element
    Item *new = malloc(sizeof(*new));

    new->value = strdup(value);
    new->key = strdup(key);
    new->next = start->next;
    start->next = new;
}

/**
 * Return index of item by value
 * @param start
 * @param value
 * @return
 */
Item * Item__getByKey(Item *start, char * key) {
    int i = 0;
    Item *item = start;

    do {
        if (item->key == NULL) continue;
        if (strcmp(item->key, key) == 0) {
            return item;
        }
    } while ((i++, item = item->next) != NULL);

    return NULL;
}

unsigned int Item__count(Item *item) {
    unsigned int count = 0;
    while (item->next != NULL && (count++, item = item->next) != NULL);
    return count;
}

void Item__print(Item *start) {
    Item *item = start;

    printf("[");

    do {
        printf("%s:%s, ", item->key, item->value);
        item = item->next;
    } while (item != NULL);

    printf("]\n");
}
