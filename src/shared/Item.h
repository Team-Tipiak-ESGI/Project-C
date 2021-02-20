typedef struct ITEM {
    char * value;
    char * key;
    struct ITEM *next;
} Item;

void Item__insert(Item *start, char * key, char * value);

Item * Item__getByKey(Item *start, char * key);

unsigned int Item__count(Item *item);

void Item__print(Item *start);