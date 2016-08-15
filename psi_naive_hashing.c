#include "psi_naive_hashing.h"

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

void psi_naive_hashing_run(PSI_NAIVE_HASHING_CTX* ctx) {
    psi_naive_hashing_show_settings(ctx);
    psi_naive_hashing_alloc_memory(ctx);
    psi_naive_hashing_handle_io(ctx);
    psi_naive_hashing_free_memory(ctx);
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
    ctx->r_buf = malloc(sizeof (*ctx->r_buf) * ctx->read_buffer_size
            * ctx->elem_size);
    ctx->w_buf = malloc(sizeof (*ctx->w_buf) * ctx->write_buffer_size
            * ctx->hash_size);
}

static void psi_naive_hashing_free_memory(PSI_NAIVE_HASHING_CTX* ctx) {
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
    int sock;
    socklen_t clientlen;
    struct sockaddr_in client_addr;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        perror("ERROR opening socket");

    int optval = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
            (const void *) &optval, sizeof (int));

    bzero((char *) &client_addr, sizeof (client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = INADDR_ANY;
    client_addr.sin_port = htons((unsigned short) ctx->port);
    if (bind(sock, (struct sockaddr *) &client_addr,
            sizeof (client_addr)) < 0) {
        perror("ERROR on binding");
        exit(EXIT_FAILURE);
    }
    if (listen(sock, 100) < 0) {
        perror("ERROR on listen");
        exit(EXIT_FAILURE);
    }
    clientlen = sizeof (client_addr);

    while (1) {
        int client_socket = accept(sock, (struct sockaddr *) &client_addr,
                &clientlen);
        if (client_socket < 0) {
            perror("ERROR on accept");
            exit(EXIT_FAILURE);
        }
        int size_accepted = 0;
        while (1) {
            size_accepted = recv(client_socket, ctx->w_buf, ctx->write_buffer_size * ctx->hash_size, 0);
            //printf("Received packet of size : %d\n", size_accepted);
            if (size_accepted > 1)
                psi_naive_hashing_write_to_file(ctx, size_accepted);
            else
                break;
        }
        if (size_accepted == 1)
            break;
    }

    close(sock);
    printf("Received elements from client\n");
    psi_naive_hashing_hash_server_elems(ctx);
}

static void psi_naive_hashing_handle_as_client(PSI_NAIVE_HASHING_CTX* ctx) {
    int sock, size_read;
    struct sockaddr_in server_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        perror("ERROR opening socket");
    int optval = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
            (const void *) &optval, sizeof (int));
    bzero((char *) &server_addr, sizeof (server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ctx->ip_str);
    server_addr.sin_port = htons((unsigned short) ctx->port);
    int error;
    if (error = connect(sock, (struct sockaddr *) &server_addr, sizeof (server_addr)) < 0) {
        printf("Error opening connection\n");
        exit(EXIT_FAILURE);
    }
    printf("Connection established\n");
    uint8_t zero[1] = {0};
    while (1) {
        size_read = fread(ctx->r_buf,
                ctx->elem_size, ctx->write_buffer_size, ctx->f_source);
        if (size_read == 0) {
            sleep(1);
            write(sock, zero, 1);
            break;
        }
        psi_naive_hashing_hash_elems(ctx, size_read);
        psi_naive_hashing_send(ctx, size_read, &sock);
    }
    close(sock);
}

static void psi_naive_hashing_write_to_file(PSI_NAIVE_HASHING_CTX* ctx, int read) {
    int written = 0;
    if (written = fwrite((ctx->w_buf), 1, read, ctx->f_dest_b) < read)
        printf("Failed to write received information to the file\n %d / %d\n", written, read);
}

static void psi_naive_hashing_send(PSI_NAIVE_HASHING_CTX* ctx, int n, int * sock) {
    write(*sock, ctx->w_buf, n * ctx->hash_size);
}

static void psi_naive_hashing_hash_elems(PSI_NAIVE_HASHING_CTX* ctx, int n) {
#pragma omp parallel for num_threads(ctx->threads) shared(ctx, n)
    for (size_t i = 0; i < n; i++)
        get_16_bit_sha256(ctx->r_buf + i * ctx->elem_size,
            ctx->w_buf + i * ctx->hash_size);
}

static void psi_naive_hashing_hash_server_elems(PSI_NAIVE_HASHING_CTX* ctx) {
    int size_read;
    while (size_read = fread(ctx->r_buf, ctx->elem_size, ctx->read_buffer_size,
            ctx->f_source)) {
        psi_naive_hashing_hash_elems(ctx, size_read);
        fwrite(ctx->w_buf, ctx->hash_size, size_read, ctx->f_dest_me);
    }
}
