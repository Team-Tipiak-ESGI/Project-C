#include <mongoc/mongoc.h>
#include <openssl/sha.h>
#include "MongoConnection.h"
#include "../shared/ChunkSize.h"

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
    bson_t *update = BCON_NEW("$pull", "{", "files", "{", "filename", BCON_UTF8(fileName), "}", "}");
    printf("Filepath: [%s]\n", fileName);

    if (!mongoc_collection_update_one(mongoConnection->collection, query, update, NULL, NULL, &error)) {
        fprintf(stderr, "%s\n", error.message);
    }
}

char * MongoConnection__listFile(MongoConnection* mongoConnection, char* username, char* password) {
    const bson_t *doc;

    // Build query
    bson_t * query = bson_new();
    BSON_APPEND_UTF8(query, "username", username);
    BSON_APPEND_UTF8(query, "password", password);

    // Query
    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(mongoConnection->collection, query, NULL, NULL);

    char * files = malloc(CHUNK_SIZE);
    files[0] = 0;

    while (mongoc_cursor_next(cursor, &doc)) {
        bson_iter_t iter;

        bson_iter_init(&iter, doc);
        bson_iter_find(&iter, "files"); // Get files sub array

        const uint8_t * data = NULL;
        uint32_t len = 0;
        bson_iter_array(&iter, &len, &data);

        bson_t * subArray = bson_new_from_data(data, len);

        // Used as actual iterator
        bson_iter_t subIter;
        bson_iter_init(&subIter, subArray);

        // Used as reference
        bson_iter_t referenceIter;
        bson_iter_init(&referenceIter, subArray);

        // Used to store values
        bson_iter_t baz;

        while (bson_iter_next(&subIter)) {
            const char * key = bson_iter_key(&subIter);

            // Build variable path
            char * path = malloc(sizeof key + sizeof ".filename");
            sprintf(path, "%s.filename", key);

            bson_iter_find_descendant(&referenceIter, path, &baz);

            const char * string = bson_iter_utf8(&baz, NULL);

            strcat(files, string);
            strcat(files, "\1");

            free(path);
        }
    }

    bson_destroy(query);
    mongoc_cursor_destroy(cursor);

    return files;
}

char * MongoConnection__getFilePath(MongoConnection* mongoConnection, char* username, char* password, const char* fileName) {
    const bson_t *doc;

    // Build query
    bson_t * query = bson_new();
    BSON_APPEND_UTF8(query, "username", username);
    BSON_APPEND_UTF8(query, "password", password);

    // Query
    mongoc_cursor_t *cursor = mongoc_collection_find_with_opts(mongoConnection->collection, query, NULL, NULL);

    while (mongoc_cursor_next(cursor, &doc)) {
        bson_iter_t iter;

        bson_iter_init(&iter, doc);
        bson_iter_find(&iter, "files"); // Get files sub array

        const uint8_t * data = NULL;
        uint32_t len = 0;
        bson_iter_array(&iter, &len, &data);

        bson_t * subArray = bson_new_from_data(data, len);

        // Used as actual iterator
        bson_iter_t subIter;
        bson_iter_init(&subIter, subArray);

        // Used as reference
        bson_iter_t referenceIter;
        bson_iter_init(&referenceIter, subArray);

        bson_iter_t referenceIter2;
        bson_iter_init(&referenceIter2, subArray);

        // Used to store values
        bson_iter_t baz, foo;

        while (bson_iter_next(&subIter)) {
            const char * key = bson_iter_key(&subIter);

            // Build variable path
            char * path = malloc(sizeof key + sizeof ".filename");
            sprintf(path, "%s.filename", key);

            bson_iter_find_descendant(&referenceIter, path, &baz);

            const char * string = bson_iter_utf8(&baz, NULL);

            if (strcmp(string, fileName) == 0) {
                sprintf(path, "%s.filepath", key);

                bson_iter_find_descendant(&referenceIter2, path, &foo);

                const char * result = bson_iter_utf8(&foo, NULL);

                return result;
            }

            free(path);
        }
    }

    bson_destroy(query);
    mongoc_cursor_destroy(cursor);

    return NULL;
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
