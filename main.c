/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: alex
 *
 * Created on June 13, 2016, 12:09 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <openssl/sha.h>

#include "psi_structures.h"
#include "psi_naive_hashing.h"
#include "psi_misc.h"


int parse_argv(int argc, char** argv, PSI_NAIVE_HASHING_CTX* ctx);
void self_check(PSI_NAIVE_HASHING_CTX* ctx);

int main(int argc, char** argv) {
    PSI_NAIVE_HASHING_CTX ctx[1];
    parse_argv(argc, argv, ctx);
    self_check(ctx);
    ctx->hash_size = SHA256_DIGEST_LENGTH;
    psi_naive_hashing(ctx);
    return (EXIT_SUCCESS);
}

int parse_argv(int argc, char** argv, PSI_NAIVE_HASHING_CTX* ctx) {
    int index, c;
    opterr = 0;
    while ((c = getopt(argc, argv, "s:d:r:w:t:e:")) != -1)
        switch (c) {
            case 'w':
                ctx->write_buffer_size = atoi(optarg);
                break;
            case 's':
                strncpy(ctx->path_source, optarg, 128);
                break;
            case 'r':
                ctx->read_buffer_size = atoi(optarg);
                break;
            case 'd':
                strncpy(ctx->path_dest, optarg, 128);
                break;
            case 'e':
                ctx->elem_size = atoi(optarg);
                break;
            case 't':
                ctx->threads = atoi(optarg);
                break;
            case '?':
                if (isprint(optopt))
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf(stderr,
                        "Unknown option character `\\x%x'.\n",
                        optopt);
                exit(EXIT_FAILURE);
            default:
                abort();
        }

    for (index = optind; index < argc; index++)
        printf("Non-option argument %s\n", argv[index]);
}

void self_check(PSI_NAIVE_HASHING_CTX * ctx){
    if(ctx->elem_size == 0){
        printf("Please define element size\n");
        exit(EXIT_FAILURE);
    }
        if(ctx->read_buffer_size == 0){
        printf("Please define read buffer size size\n");
        exit(EXIT_FAILURE);
    }
        if(ctx->write_buffer_size == 0){
        printf("Please define write buffer size\n");
        exit(EXIT_FAILURE);
    }
        if(ctx->threads == 0){
        printf("Please define number of threads\n");
        exit(EXIT_FAILURE);
    }
}
