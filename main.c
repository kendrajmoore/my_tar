//
// Created by Kendra Moore on 9/9/23.
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "create.h"
#include "helper.h"


typedef struct s_options {
    bool c;
    bool r;
    bool f;
    bool t;
    bool u;
    bool x;
    int num_of_opts;
} options_t;


options_t * get_options(char ** av)
{
    // malloc the return val
    options_t * ret = malloc(sizeof(options_t) + 1);

    // default bool false
    ret->c = false;
    ret->r = false;
    ret->f = false;
    ret->t = false;
    ret->u = false;
    ret->x = false;
    ret->num_of_opts = 0;

    for (int i = 0, j; av[i]; i++)
    {
        if (av[i][0] == '-')
        {
            for (j = 1; av[i][j]; j++)
            {
                switch (av[i][j])
                {
                    case 'c':
                        ret->c = true;
                        break;
                    case 'r':
                        ret->r = true;
                        break;
                    case 'f':
                        ret->f = true;
                        break;
                    case 't':
                        ret->t = true;
                        break;
                    case 'u':
                        ret->u = true;
                        break;
                    case 'x':
                        ret->x = true;
                        break;
                    default:
                        printf("%s: invalid input!\ninvalid option: %c\n", av[i], av[i][j]);
                }
            }
            ret->num_of_opts++;
        }
    }

    return ret;
}

int is_tar(char *input)
{
    char magic[6];
    int fd = open(input, O_RDONLY);
    lseek(fd, 257, SEEK_SET);
    int bytes_read = read(fd, magic, 6);
    close(fd);
    if(bytes_read < 6)
    {
        return 0;
    }
    if(my_strncmp(magic, "ustar", 5) == 0)
    {
        return 1;
    }
    return 0;
}


int main(int argc, char *argv[])
{
    char mode = argv[2][1];
//    printf("%c\n", mode);
    int count = argc - 2;
    char file[count][150];
    int start = 0;
    char tar_file[150];
    for(int i = 3; i < argc; i++)
    {
        if(is_tar(argv[i]))
        {
            my_strcpy(tar_file, argv[i]);

        } else {
            my_strcpy(file[start], argv[i]);
            start++;
        }
    }
    char *tar_name = argv[3];
//    printf("%s\n", tar_name);
    char **files = &argv[4];
//    printf("%s\n", *files);
    int file_count = argc - 4;
//    printf("%d\n", file_count);
    options_t * opts = get_options(argv);
    printf("\nc: %d, f: %d, r: %d, t: %d, u: %d, x: %d\n", opts->c, opts->f, opts->r, opts->t, opts->u, opts->x);
    for(int i = 0; i < start; i++)
    {
        printf("%s\n", file[i]);
    }
    switch(mode)
    {
        case 'x':
            return extract_archive(tar_name);
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
    free(opts);

//    if(argc < 2)
//    {
//        return 1;
//    }
    return 0;
}

