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

int is_tar(char *input) // change to check .tar
{
    int len = my_strlen(input);
    int min_length = 4;
    if(len < min_length)
    {
        return 0;
    }
    const char *ending = input + len -4;
    if(my_strcmp(ending, ".tar") == 0)
    {
        return 1;
    }
    return 0;
}



int main(int argc, char *argv[])
{
    char mode = argv[2][1];
    int file_count = argc - 3;
    char tar_name[150];
    int start = 0;
    if(is_tar(argv[3]))
    {
        start = 4;
        file_count -= 1;
        my_strcpy(tar_name, argv[3]);
    } else {
        start = 3;
        my_strcpy(tar_name, "default");
    }
    options_t *result = get_options(argv);

        switch(mode)
    {
        case 'x':
            return extract_archive(tar_name);
        case 'u':
            return update_archive(tar_name, &argv[start], file_count);
        case 't':
            return list_archive(tar_name);
        case 'r':
            return append_to_archive(tar_name, &argv[start], file_count);
        case 'c':
            return create_archive(tar_name, &argv[start], file_count);
        default:
            return 1;
    }

}

