#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct Hash {
    unsigned short code;
    unsigned char code_length;
} Hash;

Hash ASCII_Hash[128] = {{0, 0}};

void read_bit(FILE *file, int *bit_buffer, int *bit_count, int *bit) {
    if (*bit_count == 0) {
        *bit_buffer = fgetc(file);
        if (*bit_buffer == EOF) {
            *bit = -1;
            return;
        }
        *bit_count = 8;
    }

    *bit = (*bit_buffer >> (*bit_count - 1)) & 1;
    (*bit_count)--;
}

void read_hash_table(FILE *file) {
    while (true) {
        int symbol = fgetc(file);
        if (symbol == 0) break;

        fread(&ASCII_Hash[symbol].code, sizeof(unsigned short), 1, file);
        ASCII_Hash[symbol].code_length = fgetc(file);
    }
}

void decode_file(const char *input_filename, const char *output_filename) {
    FILE *input_file = fopen(input_filename, "rb");
    if (!input_file) {
        perror("Failed to open input file");
        exit(EXIT_FAILURE);
    }

    FILE *output_file = fopen(output_filename, "w");
    if (!output_file) {
        perror("Failed to open output file");
        exit(EXIT_FAILURE);
    }

    read_hash_table(input_file);

    int bit_buffer = 0;
    int bit_count = 0;
    int bit;
    unsigned short code = 0;
    int code_length = 0;

    while (true) {
        read_bit(input_file, &bit_buffer, &bit_count, &bit);
        if (bit == -1) {
            break;
        }

        code = (code << 1) | bit;
        code_length++;

        for (int i = 0; i < 128; i++) {
            if (ASCII_Hash[i].code == code && ASCII_Hash[i].code_length == code_length) {
                if (i == 0) {
                    fclose(input_file);
                    fclose(output_file);
                    return;
                }
                fputc(i, output_file);
                code = 0;
                code_length = 0;
                break;
            }
        }
    }

    fclose(input_file);
    fclose(output_file);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input file>\n", argv[0]);
        return 1;
    }

    char *input_filename = argv[1];

    decode_file(input_filename, "decompressed.txt");

    return 0;
}