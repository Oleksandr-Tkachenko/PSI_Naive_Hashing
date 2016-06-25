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
#include <getopt.h>

#include "psi_structures.h"
#include "psi_naive_hashing.h"
#include "psi_misc.h"


int parse_argv(int argc, char** argv, PSI_NAIVE_HASHING_CTX* ctx);
void self_check(PSI_NAIVE_HASHING_CTX* ctx);
void parse_ip(uint32_t * ip, char * s);

int main(int argc, char** argv) {
    PSI_NAIVE_HASHING_CTX ctx[1];
    parse_argv(argc, argv, ctx);
    self_check(ctx);
    ctx->hash_size = SHA256_DIGEST_LENGTH/2;
    ctx->write_buffer_size = ctx->read_buffer_size;
    psi_naive_hashing_run(ctx);
    return (EXIT_SUCCESS);
}

int parse_argv(int argc, char** argv, PSI_NAIVE_HASHING_CTX* ctx) {
    int index, c;
    opterr = 0;
    gboolean role_changed = FALSE;

    static struct option long_options[] = {
        {"port", required_argument, 0, 'p'},
        {"ip", required_argument, 0, 'i'},
        {"client", no_argument, 0, 'c'},
        {"server", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };
    ctx->role = UNDEFINED;
    /* getopt_long stores the option index here. */
    int option_index = 0;
    while ((c = getopt_long(argc, argv, "s:d:r:t:e:v:c:p:i:b:", long_options, &option_index)) != -1)
        switch (c) {
            case 's':
                strncpy(ctx->path_source, optarg, 128);
                break;
            case 'r':
                ctx->read_buffer_size = atoi(optarg);
                break;
            case 'd':
                strncpy(ctx->path_dest_me, optarg, 128);
                break;
            case 'b':
                strncpy(ctx->path_dest_b, optarg, 128);
                break;
            case 'e':
                ctx->elem_size = atoi(optarg);
                break;
            case 't':
                ctx->threads = atoi(optarg);
                break;
            case 'p':
                ctx->port = atoi(optarg);
                break;
            case 'i':
                strncpy(ctx->ip_str, optarg, 16);
                parse_ip(&ctx->ip, (char*) optarg);
                break;
            case 'c':
                if (role_changed) {
                    perror("One node can be either server or client");
                    break;
                }
                ctx->role = CLIENT;
                role_changed = TRUE;
                break;
            case 'v':
                if (role_changed) {
                    perror("One node can be either server or client");
                    break;
                }
                ctx->role = SERVER;
                role_changed = TRUE;
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
        
        if(ctx->role == SERVER)
            ctx->read_buffer_size++;
}

void self_check(PSI_NAIVE_HASHING_CTX * ctx) {
    if (ctx->elem_size == 0) {
        printf("Please set element size > 0\n");
        exit(EXIT_FAILURE);
    }
    if (ctx->read_buffer_size == 0) {
        printf("Please set read buffer size size  > 0\n");
        exit(EXIT_FAILURE);
    }
    if (ctx->threads == 0) {
        printf("Please set number of threads > 0\n");
        exit(EXIT_FAILURE);
    }
    if (ctx->role == UNDEFINED) {
        printf("Please set role of this node in network communication("
                "--server or --client)\n");
        exit(EXIT_FAILURE);
    }
    ctx->f_source = fopen(ctx->path_source, "rb");
    if (ctx->f_source == NULL) {
        printf("Error opening source file\n");
        exit(EXIT_FAILURE);
    }
    if (ctx->role == SERVER) {
        ctx->f_dest_me = fopen(ctx->path_dest_me, "wb");
        if (ctx->f_dest_me == NULL) {
            printf("Error opening destination file : %s\n", ctx->path_dest_me);
            exit(EXIT_FAILURE);
        }
        ctx->f_dest_b = fopen(ctx->path_dest_b, "wb");
        if (ctx->f_dest_b == NULL) {
            printf("Error opening destination file%s\n", ctx->path_dest_b);
            exit(EXIT_FAILURE);
        }
    }
}

void parse_ip(uint32_t * ip, char * s) {
    size_t dot = 0, count = 0;
    char tmp[3];
    if (!isdigit((int) s[0]) || strlen(s) < 7 || strlen(s) > 16) {
        printf("Incorrect IP address\nIP String length : %zu\n", strlen(s));
        exit(EXIT_FAILURE);
    }
    for (size_t i = 0; i < strlen(s) + 1; i++) {
        if (i != strlen(s) && !isdigit((int) s[i]) && s[i] != '.') break;
        else if (i == strlen(s) || s[i] == '.') {
            *ip += atoi(tmp);
            dot++;
            count = 0;
            bzero(tmp, 3);
            if (dot != 4)
                *ip <<= 8;
        } else {
            if (count == 3)
                break;
            tmp[count] = s[i];
            count++;
        }
    }
    if (dot != 4) {
        printf("Incorrect IP address\n dots = %zu\n", dot);
        exit(EXIT_FAILURE);
    }
}