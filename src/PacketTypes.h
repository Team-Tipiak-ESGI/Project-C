//
// Created by erwan on 2021-01-10.
//

#ifndef PROJECT_C_PACKETTYPES_H
#define PROJECT_C_PACKETTYPES_H

#endif //PROJECT_C_PACKETTYPES_H

enum packet_type {
    LOGIN = 0x10,
    USERNAME = 0x11,
    PASSWORD = 0x12,
    PUBKEY = 0x13,
    CREATE_USER = 0x14,
    CREATE_USERNAME = 0x15,
    CREATE_PASSWORD = 0x16,

    FILE_ACTION = 0x20,
    CREATE_FILE = 0x21,
    EDIT_FILE = 0x22,
    DELETE_FILE = 0x23,
    READ_FILE = 0x24,
    FILE_SIZE = 0x25,
    FILE_NAME = 0x26,

    FILE_CONTENT = 0x30,
    FILE_CLOSED = 0x40,
};