#include <mongoc/mongoc.h>
#include <openssl/sha.h>
#include "MongoConnection.h"

MongoConnection* MongoConnection__init(char * uri_string) {
    MongoConnection* mongoConnection = malloc(sizeof(MongoConnection));

    mongoConnection->client = mongoc_client_new(uri_string);
    mongoConnection->collection = mongoc_client_get_collection(mongoConnection->client, "TUCS", "users");

    return mongoConnection;
}

int MongoConnection__getUser(MongoConnection* mongoConnection, char* username, char* password) {
    // Build query
    bson_t * query = bson_new();

    BSON_APPEND_UTF8(query, "username", username);

    if (password != NULL) {
        BSON_APPEND_UTF8(query, "password", password);
    }

    bson_error_t error;
    int count = mongoc_collection_count(mongoConnection->collection, MONGOC_QUERY_NONE, query, 0, 0, NULL, &error);

    if (count < 0) {
        fprintf(stderr, "%s\n", error.message);
    }

    bson_destroy(query);

    return count;
}

int MongoConnection__createUser(MongoConnection* mongoConnection, char* username, char* password) {
    if (MongoConnection__getUser(mongoConnection, username, NULL) != 0) {
        return 0;
    }

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

    return 1;
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
