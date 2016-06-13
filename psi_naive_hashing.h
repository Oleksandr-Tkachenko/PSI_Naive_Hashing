/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   psi_naive_hashing.h
 * Author: alex
 *
 * Created on June 13, 2016, 12:24 PM
 */

#ifndef PSI_NAIVE_HASHING_H
#define PSI_NAIVE_HASHING_H

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "psi_misc.h"
#include "psi_structures.h"
#include "psi_hashing.h"

#define SLEEP_NOTIF_THRESHOLD 10

#ifdef __cplusplus
extern "C" {
#endif

    void psi_naive_hashing_run(PSI_NAIVE_HASHING_CTX * ctx);
    static void psi_naive_hashing_show_settings(PSI_NAIVE_HASHING_CTX* ctx);
    static void psi_naive_hashing_alloc_memory(PSI_NAIVE_HASHING_CTX* ctx);
    static void psi_naive_hashing_free_memory(PSI_NAIVE_HASHING_CTX* ctx);
    static void psi_naive_hashing_handle_io(PSI_NAIVE_HASHING_CTX* ctx);
    static void psi_naive_hashing_handle_as_server(PSI_NAIVE_HASHING_CTX* ctx);
    static void psi_naive_hashing_handle_as_client(PSI_NAIVE_HASHING_CTX* ctx);
    static void psi_naive_hashing_write_to_file(PSI_NAIVE_HASHING_CTX* ctx, int read);
    static void psi_naive_hashing_send(PSI_NAIVE_HASHING_CTX* ctx, int n, int * sock);
    static void psi_naive_hashing_hash_elems(PSI_NAIVE_HASHING_CTX* ctx, int n);
    static void psi_naive_hashing_hash_server_elems(PSI_NAIVE_HASHING_CTX* ctx);

#ifdef __cplusplus
}
#endif

#endif /* PSI_NAIVE_HASHING_H */

