//
// Created by Kendra Moore on 9/9/23.

#include "create.h"

int main(int argc, char *argv[])
{

    char mode = argv[2][1];
//    printf("%c\n", mode);
    char *tar_name = argv[3];
//    printf("%s\n", tar_name);
    char **files = &argv[4];
//    printf("%s\n", *files);
    int file_count = argc - 4;
//    printf("%d\n", file_count);
    switch(mode)
    {
        case 'u':
            return update_archive(tar_name, files, file_count);
        case 't':
            return list_archive(tar_name);
        case 'r':
            return append_to_archive(tar_name, files, file_count);
        case 'c':
            return create_archive(tar_name, files, file_count);
        default:
            return 1;
    }
//    if(argc < 2)
//    {
//        return 1;
//    }
    return 0;
}

