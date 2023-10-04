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


options_t * get_options(char ** av){
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

    for (int i = 0, j; av[i]; i++){
        if (av[i][0] == '-'){
            for (j = 1; av[i][j]; j++){
                switch (av[i][j]){
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

//function to check for -.tar as a parameter
int is_tar(char *input) {
    int len = my_strlen(input);
    int min_length = 4;
    if(len < min_length){
        return 0;
    }
    const char *ending = input + len -4;
    if(my_strcmp(ending, ".tar") == 0){
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[]){
    int file_count = argc - 2;
    char tar_name[150];
    int start = 0;
    if(is_tar(argv[2])){
        start = 3;
        file_count -= 1;
        my_strcpy(tar_name, argv[2]);
    } else {
        start = 2;
        my_strcpy(tar_name, "default");
    }
    options_t *result = get_options(argv);
    if(result->c){

        return create_archive(tar_name, &argv[start], file_count);
    }
    if(result->r && result->f){
        return append_to_archive(tar_name, &argv[start], file_count);
    }
    if(result->u && result->f){
        return update_archive(tar_name, &argv[start], file_count);
    }
    if(result->t){
        return list_archive(tar_name);
    }
    if(result->x){
        return extract_archive(tar_name);
    }
    else {
        write_stderr("Error reading the mode or incorrect combination of options provided.");
        return 1;
    }

}


