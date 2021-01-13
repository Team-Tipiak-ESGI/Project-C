typedef struct {
    char* username;
    char* password;
    FILE* file;
    int fileSize;
    int chunkSent;
    char* hashedFileName;
} Client;
