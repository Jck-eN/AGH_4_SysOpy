// Autor: Jacek Nitychoruk
#ifndef DIFF_LIBRARY_H
#define DIFF_LIBRARY_H


struct operations_block{
    int operations_count;
    char** operations;
};

struct main_arr{
    int blocks_size;
    struct operations_block** blocks;
    int last_block;
};

struct files_pair{
    char* first_path;
    char* second_path;
};

struct pairs_sequence{
    int pairs_number;
    struct files_pair** pairs;
};

// 1. Utworzenie tablicy wskaźników (tablicy głównej)
struct main_arr* create_main_arr(int blocks_size);
// 2. Definiowanie sekwencji par plików
struct pairs_sequence* create_files_sequence(int pairs_count, char** pairs);
// 3. Przeprowadzenie porównania (dla każdego elementu sekwencji) oraz zapisanie wyniku porównania do pliku tymczasowego
void compare_sequence(struct main_arr* arr, struct pairs_sequence* sequence);
// 4. Utworzenie, na podstawie zawartość pliku tymczasowego, bloku operacji edycyjnych (...) funkcja powinna zwrócić indeks elementu tablicy (głównej), który zawiera wskazanie na utworzony blok.
int add_block (struct main_arr* arr, char* tmp_filename);
// 5. Zwrócenie informacji o ilości operacji w danym bloku operacji edycyjnych
int block_size(struct main_arr* arr, int block_idx);
// 6. Usunięcie, z pamięci, bloku (operacji edycyjnych) o zadanym indeksie
void remove_block(struct main_arr* arr, int block_idx_to_delete);
// 7. Usunięcie, z pamięci, określonej operacji dla podanego bloku operacji edycyjnych.
void remove_operation(struct operations_block* block, int op_idx_to_delete);

// Funkcje pomocnicze:
// Do 2
struct files_pair* create_files_pair(char* paths);
// Do 3
void compare_files_and_save_to_file(struct files_pair* pair, char* filename);
// Do 4
struct operations_block* create_block (char* tmp_filename);


#endif