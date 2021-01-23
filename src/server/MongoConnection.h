#include <mongoc.h>

typedef struct {
    mongoc_uri_t *uri;
    mongoc_client_t *client;
    mongoc_database_t *database;
    mongoc_collection_t *collection;
} MongoConnection;
