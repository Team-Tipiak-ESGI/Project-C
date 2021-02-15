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

void MongoConnection__getUser(MongoConnection* mongoConnection, char* username, char* password) {
    // Build query
    bson_t * query = bson_new();

    BSON_APPEND_UTF8(query, "username", username);
    BSON_APPEND_UTF8(query, "password", password);

    // Make query
    mongoc_cursor_t * cursor = mongoc_collection_find_with_opts (mongoConnection->collection, query, NULL, NULL);

    // Output
    const bson_t *doc;

    while (mongoc_cursor_next (cursor, &doc)) {
        char * str = bson_as_canonical_extended_json(doc, NULL);
        printf("%s\n", str);
        bson_free(str);
    }

    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
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
