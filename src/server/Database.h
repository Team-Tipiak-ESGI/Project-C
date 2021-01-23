#include <mongoc.h>
#include "MongoConnection.h"

MongoConnection* MongoConnection__init();
void MongoConnection__close(MongoConnection* mongoConnection);
