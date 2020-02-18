//
// Created by mikolajsikora on 12.03.19.
//

#ifndef SYSTEMYOP1_LIBRARY_H
#define SYSTEMYOP1_LIBRARY_H

struct block_arr {
    int number_of_blocks;
    char **blocks;
};

struct block_arr *create_array (int number_of_blocks);

void delete_block_at_index (struct block_arr *arr, int index);

int reserve_block (struct block_arr *arr, char * tmp_file);

void search_file (char * tmp_file);

void set_dir_file (char * directory, char * file);

void prepare_dir_file ();

#endif //SYSTEMYOP1_LIBRARY_H
