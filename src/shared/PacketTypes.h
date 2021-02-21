enum packet_type {
    // Login packets
    LOGIN = 0x10,
    USERNAME = 0x11,
    PASSWORD = 0x12,
    PUBKEY = 0x13,
    CREATE_USER = 0x14, // Sent by the client after sending USERNAME and PASSWORD to create a new account

    // File actions packets
    FILE_ACTION = 0x20,
    CREATE_FILE = 0x21, // Sent by the client to create a new file, its content is defined with the FILE_CONTENT packet
    EDIT_FILE = 0x22,
    DELETE_FILE = 0x23, // Sent by the client to request a specific file to be deleted
    READ_FILE = 0x24, // Sent by the client to download a file
    FILE_SIZE = 0x25, // Packet containing the size of the file
    FILE_NAME = 0x26, // Packet containing the name of the file

    FILE_CONTENT = 0x30, // Packet containing the content of the file
    FILE_CLOSED = 0x40,

    HTTP = 72, // First byte of HTTP request is a H (ascii 72)

    // Status packets
    CHUNK_RECEIVED = 0x50, // Sent as a confirmation by the server when receiving file chunks
    UNAUTHORIZED = 0x60, // Sent if the client is not logged in
    USER_CREATED = 0x61, // Sent if the new user was successfully created
    USER_EXISTS = 0x62, // Sent when creating a new user if the username is already taken
    FILE_DELETED = 0x63,
    FILE_CREATED = 0x64,
    LOGGED_IN = 0x65,
    USERNAME_RECEIVED = 0x66,
    PASSWORD_RECEIVED = 0x67,
    DELETE_ERROR = 0x68,

    LIST_FILES = 0x70, // Sent by the client to request a list of all the files currently held by the server
};