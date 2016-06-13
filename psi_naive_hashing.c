#include "psi_naive_hashing.h"

void psi_naive_hashing_run(PSI_NAIVE_HASHING_CTX* ctx) {
    psi_naive_hashing_show_settings(ctx);
    psi_naive_hashing_alloc_memory(ctx);
    psi_naive_hashing_free_memory(ctx);
    psi_naive_hashing_handle_io(ctx);
}

static void psi_naive_hashing_show_settings(PSI_NAIVE_HASHING_CTX* ctx) {
    printf("\n");
    if (ctx->role == CLIENT)printf("I'm a client node\n");
    else printf("I'm a server node\n");
    printf("Reading from : %s\n", ctx->path_source);
    if (ctx->role == SERVER) {
        printf("Writing my data to : %s\n", ctx->path_dest_me);
        printf("Writing b's data to : %s\n", ctx->path_dest_b);
    }
    printf("Element size : %zu\n", ctx->elem_size);
    printf("Hash size : %zu\n", ctx->hash_size);
    printf("Read buffer size : %zu\n", ctx->read_buffer_size);
    printf("Write buffer size : %zu\n", ctx->write_buffer_size);
    printf("Threads : %zu\n", ctx->threads);

    printf("Destination IP : %u.%u.%u.%u\n", (ctx->ip >> 24) & 0xFF,
            (ctx->ip >> 16) & 0xFF, (ctx->ip >> 8) & 0xFF, ctx->ip & 0xFF);
    printf("Destination Port : %zu\n", ctx->port);
    printf("\n");
}

static void psi_naive_hashing_alloc_memory(PSI_NAIVE_HASHING_CTX* ctx) {
    ctx->r_buf = (uint8_t**) malloc(sizeof (*ctx->r_buf) * 2);
    ctx->w_buf = (uint8_t**) malloc(sizeof (*ctx->w_buf) * 2);
    for (size_t i = 0; i < 2; i++) {

        ctx->r_buf[i] = malloc(sizeof (*ctx->r_buf[i]) * ctx->read_buffer_size
                * ctx->elem_size);
        ctx->w_buf[i] = malloc(sizeof (*ctx->w_buf[i]) * ctx->write_buffer_size
                * ctx->hash_size);
    }
}

static void psi_naive_hashing_free_memory(PSI_NAIVE_HASHING_CTX* ctx) {
    for (size_t i = 0; i < 2; i++) {

        free(ctx->r_buf[i]);
        free(ctx->w_buf[i]);
    }
    free(ctx->r_buf);
    free(ctx->w_buf);
}

static void psi_naive_hashing_handle_io(PSI_NAIVE_HASHING_CTX* ctx) {
    if (ctx->role == SERVER)
        psi_naive_hashing_handle_as_server(ctx);
    else
        psi_naive_hashing_handle_as_client(ctx);
}

static void psi_naive_hashing_handle_as_server(PSI_NAIVE_HASHING_CTX* ctx) {
    int sock, clientlen, size_accepted, size_read, tmp_read;
    struct sockaddr_in client_addr;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        perror("ERROR opening socket");

    int optval = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
            (const void *) &optval, sizeof (int));

    bzero((char *) &client_addr, sizeof (client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htonl(ctx->ip);
    client_addr.sin_port = htons((unsigned short) ctx->port);
    if (bind(sock, (struct sockaddr *) &client_addr,
            sizeof (client_addr)) < 0) {
        perror("ERROR on binding");
        exit(EXIT_FAILURE);
    }
    if (listen(sock, 10) < 0) {
        perror("ERROR on listen");
        exit(EXIT_FAILURE);
    }
    clientlen = sizeof (client_addr);
#pragma omp parallel num_threads(ctx->threads) shared(ctx, sock, clientlen, client_addr, size_accepted, size_read, tmp_read) 
    {
#pragma omp single
        {
            size_t sleep_counter = 0;
            while (1) {
                if (size_accepted = accept(sock, (struct sockaddr *) &client_addr,
                        &clientlen) < 0) {
                    perror("ERROR on accept");
                    exit(EXIT_FAILURE);
                }
                size_read = read(size_accepted, ctx->w_buf[ctx->actual_array % 2],
                        ctx->write_buffer_size * ctx->hash_size);

                if (size_read < 0) {
                    perror("ERROR reading from socket");
                    break;
                } else if (size_read == 1)
                    break;
                while (ctx->writing_flag);
                ctx->actual_array++;
                tmp_read = size_read;
#pragma omp task
                {

                    psi_naive_hashing_write_to_file(ctx, tmp_read);
                }
            }
        }
    }

    close(sock);
    printf("Received elements from client\n");
    psi_naive_hashing_hash_server_elems(ctx);
}

static void psi_naive_hashing_handle_as_client(PSI_NAIVE_HASHING_CTX* ctx) {
    int sock, size_sent, size_read;
    struct sockaddr_in server_addr;
    gboolean sending = FALSE;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        perror("ERROR opening socket");
    int optval = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
            (const void *) &optval, sizeof (int));
    bzero((char *) &server_addr, sizeof (server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(ctx->ip);
    server_addr.sin_port = htons((unsigned short) ctx->port);
    if (bind(sock, (struct sockaddr *) &server_addr,
            sizeof (server_addr)) < 0) {
        perror("ERROR on binding");
        exit(EXIT_FAILURE);
    }
    int sleep_counter = 0;
    while (connect(sock, (struct sockaddr *) &server_addr,
            sizeof (server_addr) < 0)) {
        if (sleep_counter == 0) {
            sleep_counter++;
            printf("Waiting for connection\n");
        }
        sleep(1);
    }
    printf("Connection established\n");
#pragma omp parallel num_threads(ctx->threads) shared(ctx, sock, size_sent,size_read, server_addr, sending) 
    {
#pragma omp single
        {
            uint8_t zero[1] = {0};
            while (1) {
                size_read = fread(ctx->r_buf[ctx->actual_array % 2],
                        ctx->elem_size, ctx->write_buffer_size, ctx->f_source);
                if (size_read == 0) {
                    write(sock, zero, 1);
                    break;
                }
                psi_naive_hashing_hash_elems(ctx, size_read);
                while (ctx->writing_flag);
                size_sent = size_read;
                ctx->actual_array++;
#pragma omp task
                {
                    psi_naive_hashing_send(ctx, size_sent, &sock);
                }
            }
        }
    }
    close(sock);
}

static void psi_naive_hashing_write_to_file(PSI_NAIVE_HASHING_CTX* ctx, int read) {

    while (ctx->writing_flag);
    ctx->writing_flag = TRUE;
    fwrite((ctx->w_buf[(ctx->actual_array + 1) % 2]), read, 1, ctx->f_dest_b);
    ctx->writing_flag = FALSE;
}

static void psi_naive_hashing_send(PSI_NAIVE_HASHING_CTX* ctx, int n, int * sock) {

    ctx->writing_flag = TRUE;
    write(*sock, ctx->w_buf[(ctx->actual_array + 1) % 2], 19 * n);
    ctx->writing_flag = FALSE;
}

static void psi_naive_hashing_hash_elems(PSI_NAIVE_HASHING_CTX* ctx, int n) {
#pragma omp parallel for num_threads(ctx->threads) shared(ctx, n)

    for (size_t i = 0; i < n; i++)
        get_sha256(ctx->r_buf[ctx->actual_array % 2] + i * ctx->elem_size,
            ctx->w_buf[ctx->actual_array % 2] + i * ctx->hash_size);
}

static void psi_naive_hashing_hash_server_elems(PSI_NAIVE_HASHING_CTX* ctx) {
    int size_read;
    ctx->actual_array = 0;
    while (size_read = fread(ctx->r_buf[0], ctx->elem_size, ctx->elem_size,
            ctx->f_source)) {
        psi_naive_hashing_hash_elems(ctx, size_read);
        fwrite(ctx->w_buf[0], ctx->hash_size, size_read, ctx->f_dest_me);
    }
}