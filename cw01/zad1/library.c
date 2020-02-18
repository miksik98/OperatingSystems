//
// Created by mikolajsikora on 12.03.19.
//

#include "library.h"
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>


struct block_arr *create_array (int number_of_blocks){

    if (number_of_blocks < 0){
        return NULL;
    }

    struct block_arr * new_arr = malloc(sizeof(struct block_arr));
    new_arr->number_of_blocks = number_of_blocks;
    char **blocks = (char **) calloc(number_of_blocks, sizeof(char *));
    new_arr->blocks = blocks;

    return new_arr;
}

void delete_block_at_index (struct block_arr *arr, int index){
    if (arr == NULL || arr->blocks[index] == NULL) return;
    free(arr->blocks[index]);
    arr->blocks[index] = NULL;
}

int reserve_block (struct block_arr *arr, char * tmp_file){
    int index = -1;
    for (int i = 0; i < arr->number_of_blocks; i++){
        if (arr->blocks[i] == NULL){
            index = i;
            break;
        }
    }

    if (index > -1) {
        FILE *file = fopen(tmp_file, "r");
        fseek(file, 0, SEEK_END);
        int tmp_size = ftell(file);
        arr->blocks[index] = malloc(tmp_size * sizeof(char));
        fseek(file, 0, SEEK_SET);
        fread(arr->blocks[index], tmp_size, 1, file);
    }
    return index;
}

void search_file (char * tmp_file){
    char command[200];
    strcpy(command, "find ");
    strcat(command, directory);
    strcat(command, " -name \"");
    strcat(command, file);
    strcat(command, "\" > ");
    strcat(command, tmp_file);
    system(command);
}

void set_dir_file (char * directory_given, char * file_given){
    strcpy(directory,directory_given);
    strcpy(file,file_given);
}
