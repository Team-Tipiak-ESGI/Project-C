#include <mongoc.h>

MongoConnection* MongoConnection__init();
void MongoConnection__close(MongoConnection* mongoConnection);
