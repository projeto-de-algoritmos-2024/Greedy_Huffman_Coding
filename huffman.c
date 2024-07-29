#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef char Symbol;

int count_symbols = 0;

typedef struct Node {
    Symbol symbol;
    int recorrence;
    struct Node *left, *right;
} Node;

Node *create_node(Symbol symbol) {
    Node *new_node = malloc(sizeof(Node));
    new_node->left = NULL;
    new_node->right = NULL;
    new_node->symbol = symbol;
    new_node->recorrence = 1;
    return new_node;
}

typedef struct Tree {
    int weight;
    Node *left, *right;
} Tree;

typedef struct Hash {
    Node *node_address;
    unsigned short code;
    unsigned char code_length;
} Hash;

Hash ASCII_Hash[128] = {{NULL, 0, 0}};

void insert_symbol(Symbol symbol) {
    int index = (int)symbol;
    if (ASCII_Hash[index].node_address == NULL) {
        Node *new_node = create_node(symbol);
        ASCII_Hash[index].node_address = new_node;
        count_symbols++;
        return;
    }
    Node *node = ASCII_Hash[index].node_address;
    node->recorrence++;
    return;
}

typedef struct pq_st {
    Tree *pq;
    int N;
} pq_st;

bool less(Tree a, Tree b) {
    return a.weight > b.weight;
}

void exch(Tree *a, Tree *b) {
    Tree temp = *a;
    *a = *b;
    *b = temp;
}

void initPQ(int maxN, pq_st *PQ) {
    PQ->pq = malloc(sizeof(Tree) * (maxN + 1));
    PQ->N = 0;
}

int PQempty(pq_st *PQ) {
    return PQ->N == 0;
}

void fixDown(Tree *v, int k, int n) {
    int j;
    while (2 * k <= n) {
        j = 2 * k;
        if (j < n && less(v[j], v[j + 1])) j++;
        if (!less(v[k], v[j])) break;
        exch(&v[k], &v[j]);
        k = j;
    }
}

void fixUp(Tree *v, int k) {
    while (k > 1 && less(v[k / 2], v[k])) {
        exch(&v[k], &v[k / 2]);
        k = k / 2;
    }
}

void PQinsert(pq_st *PQ, Tree novo) {
    PQ->pq[++PQ->N] = novo;
    fixUp(PQ->pq, PQ->N);
}

Tree PQdelMax(pq_st *PQ) {
    exch(&PQ->pq[1], &PQ->pq[PQ->N]);
    fixDown(PQ->pq, 1, PQ->N - 1);
    return PQ->pq[PQ->N--];
}

void sow_tree(pq_st *PQ) {
    for (int i = 0; i < 128; i++) {
        if (ASCII_Hash[i].node_address != NULL) {
            Tree new_tree;
            new_tree.weight = ASCII_Hash[i].node_address->recorrence;
            new_tree.left = ASCII_Hash[i].node_address;
            new_tree.right = NULL;
            PQinsert(PQ, new_tree);
        }
    }
}

Node *create_null_node(Node *left, Node *right) {
    Node *null_node = malloc(sizeof(Node));
    null_node->symbol = '\0';
    null_node->recorrence = left->recorrence + right->recorrence;
    null_node->left = left;
    null_node->right = right;

    return null_node;
}

Tree join_tree(Tree forest_1, Tree forest_2) {
    Node *new_root = create_null_node(forest_1.left, forest_2.left);
    Tree new_tree;
    new_tree.weight = forest_1.weight + forest_2.weight;
    new_tree.left = new_root;
    new_tree.right = NULL;
    return new_tree;
}

void generate_codes(Node *root, unsigned short code, int length) {
    if (root->left != NULL) {
        generate_codes(root->left, code << 1, length + 1);
    }

    if (root->right != NULL) {
        generate_codes(root->right, (code << 1) | 1, length + 1);
    }

    if (root->left == NULL && root->right == NULL) {
        int index = (int)root->symbol;
        ASCII_Hash[index].code = code;
        ASCII_Hash[index].code_length = length;
    }
}

void write_bit(FILE *file, int bit, int *bit_buffer, int *bit_count) {
    *bit_buffer = (*bit_buffer << 1) | bit;
    (*bit_count)++;
    if (*bit_count == 8) {
        fputc(*bit_buffer, file);
        *bit_count = 0;
        *bit_buffer = 0;
    }
}

void write_compressed_file(const char *filename, const char *text) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Failed to open file for writing");
        exit(EXIT_FAILURE);
    }

    // Escreve a tabela hash no início do arquivo
    for (int i = 0; i < 128; i++) {
        if (ASCII_Hash[i].node_address != NULL) {
            fputc(i, file); // Símbolo
            fwrite(&ASCII_Hash[i].code, sizeof(unsigned short), 1, file); // Código
            fputc(ASCII_Hash[i].code_length, file); // Comprimento do código
        }
    }
    fputc(0, file); // Delimitador de fim da tabela hash

    int bit_buffer = 0;
    int bit_count = 0;

    for (size_t i = 0; text[i] != '\0'; i++) {
        unsigned short code = ASCII_Hash[(int)text[i]].code;
        unsigned char code_length = ASCII_Hash[(int)text[i]].code_length;
        for (int j = code_length - 1; j >= 0; j--) {
            int bit = (code >> j) & 1;
            write_bit(file, bit, &bit_buffer, &bit_count);
        }
    }

    if (bit_count > 0) {
        bit_buffer <<= (8 - bit_count);
        fputc(bit_buffer, file);
    }

    fclose(file);
}

char *read_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *buffer = malloc(length + 1);
    if (buffer) {
        fread(buffer, 1, length, file);
        buffer[length] = '\0';
    }

    fclose(file);
    return buffer;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input file>\n", argv[0]);
        return 1;
    }

    char *input_filename = argv[1];
    char *text = read_file(input_filename);

    for (int i = 0; text[i] != '\0'; i++) {
        insert_symbol(text[i]);
    }

    pq_st PQ;
    initPQ(count_symbols, &PQ);

    sow_tree(&PQ);

    Tree forest_1;
    Tree forest_2;
    Tree result_tree;

    while (PQ.N > 1) {
        forest_1 = PQdelMax(&PQ);
        forest_2 = PQdelMax(&PQ);

        result_tree = join_tree(forest_1, forest_2);

        PQinsert(&PQ, result_tree);
    }

    Node *root = result_tree.left;

    generate_codes(root, 0, 0);

    char output_filename[256];
    snprintf(output_filename, sizeof(output_filename), "%s_compressed.bin", input_filename);

    write_compressed_file(output_filename, text);
    
    free(text);
    free(PQ.pq);
    return 0;
}