#include <mongoc/mongoc.h>
#include "MongoConnection.h"

MongoConnection* MongoConnection__init() {
    const char *uri_string = "mongodb://localhost:27017";

    MongoConnection* mongoConnection = malloc(sizeof(MongoConnection));

    bson_t *command, reply, *insert;
    bson_error_t error;
    char *str;
    bool retval;

    /*
     * Required to initialize libmongoc's internals
     */
    mongoc_init();

    /*
     * Safely create a MongoDB URI object from the given string
     */
    mongoConnection->uri = mongoc_uri_new_with_error(uri_string, &error);
    if (!mongoConnection->uri) {
        fprintf(stderr,
                "failed to parse URI: %s\n"
                "error message:       %s\n",
                uri_string,
                error.message);
        return NULL;
    }

    /*
     * Create a new client instance
     */
    mongoConnection->client = mongoc_client_new_from_uri(mongoConnection->uri);
    if (!mongoConnection->client) {
        return NULL;
    }

    /*
     * Register the application name so we can track it in the profile logs
     * on the server. This can also be done from the URI (see other examples).
     */
    mongoc_client_set_appname(mongoConnection->client, "connect-example");

    /*
     * Get a handle on the database "db_name" and collection "coll_name"
     */
    mongoConnection->database = mongoc_client_get_database(mongoConnection->client, "db_name");
    mongoConnection->collection = mongoc_client_get_collection(mongoConnection->client, "db_name", "coll_name");

    /*
     * Do work. This example pings the database, prints the result as JSON and
     * performs an insert
     */
    command = BCON_NEW ("ping", BCON_INT32(1));

    retval = mongoc_client_command_simple(mongoConnection->client, "admin", command, NULL, &reply, &error);

    if (!retval) {
        fprintf(stderr, "%s\n", error.message);
        return NULL;
    }

    str = bson_as_json(&reply, NULL);
    printf("%s\n", str);

    insert = BCON_NEW ("hello", BCON_UTF8("world"));

    if (!mongoc_collection_insert_one(mongoConnection->collection, insert, NULL, NULL, &error)) {
        fprintf(stderr, "%s\n", error.message);
    }

    bson_destroy(insert);
    bson_destroy(&reply);
    bson_destroy(command);
    bson_free(str);

    return mongoConnection;
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
