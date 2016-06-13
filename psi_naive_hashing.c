#include "psi_naive_hashing.h"

void psi_naive_hashing(PSI_NAIVE_HASHING_CTX* ctx) {
    show_settings();
}

void show_settings(PSI_NAIVE_HASHING_CTX* ctx) {
    printf("Reading from : %zu\n", ctx->path_root);
    printf("Writing to : %zu\n", ctx->path_dest);
    printf("Element size : %zu\n", ctx->elem_size);
    printf("Hash size : %zu\n", ctx->hash_size);
    printf("Read buffer size : %zu\n", ctx->read_buffer_size);
    printf("Write buffer size : %zu\n", ctx->write_buffer_size);
    printf("Threads : %zu\n", ctx->threads);
    printf("\n");
}