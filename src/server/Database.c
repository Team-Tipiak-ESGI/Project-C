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
    // If a user with the given username already exists, abort
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

void MongoConnection__addFile(MongoConnection* mongoConnection, char* username, char* password, const char* fileName, char* filePath) {
    bson_error_t error;

    // Build query
    bson_t * query = bson_new();
    BSON_APPEND_UTF8(query, "username", username);
    BSON_APPEND_UTF8(query, "password", password);

    // Build update
    bson_t *update = BCON_NEW("$push", "{", "files", "{", "filename", BCON_UTF8(fileName), "filepath", BCON_UTF8(filePath), "}", "}");

    if (!mongoc_collection_update_one(mongoConnection->collection, query, update, NULL, NULL, &error)) {
        fprintf(stderr, "%s\n", error.message);
    }
}

void MongoConnection__deleteFile(MongoConnection* mongoConnection, char* username, char* password, const char* fileName) {
    bson_error_t error;

    // Build query
    bson_t * query = bson_new();
    BSON_APPEND_UTF8(query, "username", username);
    BSON_APPEND_UTF8(query, "password", password);

    // Build update
    // TODO: Replace filepath with filename
    bson_t *update = BCON_NEW("$pull", "{", "files", "{", "filepath", BCON_UTF8(fileName), "}", "}");
    printf("Filepath: [%s]\n", fileName);

    if (!mongoc_collection_update_one(mongoConnection->collection, query, update, NULL, NULL, &error)) {
        fprintf(stderr, "%s\n", error.message);
    }
}

void MongoConnection__listFile(MongoConnection* mongoConnection, char* username, char* password) {
    bson_error_t error;
    const bson_t *doc;

    // Build query
    bson_t * query = bson_new();
    BSON_APPEND_UTF8(query, "username", username);
    BSON_APPEND_UTF8(query, "password", password);

    // Query
    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(mongoConnection->collection, query, NULL, NULL);

    while (mongoc_cursor_next(cursor, &doc)) {
        bson_iter_t iter;
        bson_type_t type;

        bson_iter_init(&iter, doc);

        bson_iter_find(&iter, "files");

        const uint8_t * data = NULL;
        uint32_t len = 0;
        bson_iter_array(&iter, &len, &data);

        bson_t * fSubArray = bson_new_from_data(data, len);
        bson_iter_t fIter;
        bson_iter_init(&fIter, fSubArray);

        while ((type = bson_iter_next(&fIter))) {
            printf("Type: %d Key: [%s]\n", type, bson_iter_key(&fIter));

            /*bson_iter_t sub;
            bool a = bson_iter_find_descendant(&fIter, "filename", &sub);
            printf("a: %d Address: %p\n", a, &sub);*/

            uint32_t length = 0;
            const char * value = bson_iter_utf8(&fIter, NULL);
            printf("Value: [%s] Length: %d\n", value, length);
        }
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
