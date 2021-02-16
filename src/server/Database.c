#include <mongoc/mongoc.h>
#include <bson/bson.h>
#include "MongoConnection.h"

MongoConnection* MongoConnection__init() {
    const char *uri_string = "mongodb://localhost:27017";

    MongoConnection* mongoConnection = malloc(sizeof(MongoConnection));

    mongoConnection->client = mongoc_client_new(uri_string);
    mongoConnection->collection = mongoc_client_get_collection(mongoConnection->client, "TUCS", "users");

    return mongoConnection;
}

void MongoConnection__createUser(MongoConnection* mongoConnection, char* username, char* password) {
    bson_t * document = bson_new();
    bson_error_t error;

    BSON_APPEND_UTF8(document, "username", username);
    BSON_APPEND_UTF8(document, "password", password);

    char * str = bson_as_canonical_extended_json(document, NULL);
    printf("%s\n", str);
    bson_free(str);

    if (!mongoc_collection_insert_one(mongoConnection->collection, document, NULL, NULL, &error)) {
        fprintf(stderr, "%s\n", error.message);
    }

    bson_destroy(document);
}

int MongoConnection__getUser(MongoConnection* mongoConnection, char* username, char* password) {
    // Build query
    bson_t * query = bson_new();

    BSON_APPEND_UTF8(query, "username", username);
    BSON_APPEND_UTF8(query, "password", password);

    bson_error_t error;
    int count = mongoc_collection_count(mongoConnection->collection, MONGOC_QUERY_NONE, query, 0, 0, NULL, &error);

    if (count < 0) {
        fprintf (stderr, "%s\n", error.message);
    } else {
        printf ("%d\n", count);
    }

    bson_destroy(query);

    return count;
}

/**
 * Release our handles and clean up libmongoc
 */
void MongoConnection__close(MongoConnection* mongoConnection) {
    mongoc_collection_destroy(mongoConnection->collection);
    mongoc_database_destroy(mongoConnection->database);
    mongoc_uri_destroy(mongoConnection->uri);
    mongoc_client_destroy(mongoConnection->client);
    mongoc_cleanup();
}
