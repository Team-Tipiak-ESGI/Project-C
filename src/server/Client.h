typedef struct {
    char* username;
    char* password;
    char* filePath;
    FILE* file;
    int fileSize;
    int chunkSent;
    int chunkSize;
} Client;
