#include "socket_driver.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#define LVGL_SOCKET_SEND_TIMEOUT_MS 1000
#define LVGL_SOCKET_RECV_TIMEOUT_MS 1000

int lvgl_socket_send(const char *msg) {
    int sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock == -1) {
        perror("socket");
        return -1;
    }

    struct timeval timeout = {
        .tv_sec = LVGL_SOCKET_SEND_TIMEOUT_MS / 1000,
        .tv_usec = (LVGL_SOCKET_SEND_TIMEOUT_MS % 1000) * 1000
    };
    if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) == -1) {
        perror("setsockopt SO_SNDTIMEO");
        close(sock);
        return -1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, LVGL_READ_SOCKET_PATH, sizeof(addr.sun_path) - 1);

    ssize_t sent = sendto(sock, msg, strlen(msg), 0,
                          (struct sockaddr*)&addr, sizeof(addr));
    if (sent == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            fprintf(stderr, "sendto timed out after %d ms\n", LVGL_SOCKET_SEND_TIMEOUT_MS);
        } else {
            perror("sendto");
        }
        close(sock);
        return -1;
    }

    close(sock);
    return 0;
}

int lvgl_socket_recv_timeout(char *buf, size_t bufsize, int timeout_ms) {
    int sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock == -1) {
        perror("socket");
        return -1;
    }

    struct timeval timeout = {
        .tv_sec = timeout_ms / 1000,
        .tv_usec = (timeout_ms % 1000) * 1000
    };
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1) {
        perror("setsockopt SO_RCVTIMEO");
        close(sock);
        return -1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, LVGL_WRITE_SOCKET_PATH, sizeof(addr.sun_path) - 1);

    // Remove o socket antigo, se existir
    unlink(LVGL_WRITE_SOCKET_PATH);
    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close(sock);
        return -1;
    }

    ssize_t recvd = recvfrom(sock, buf, bufsize - 1, 0, NULL, NULL);
    if (recvd == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            fprintf(stderr, "recvfrom timed out after %d ms\n", timeout_ms);
        } else {
            perror("recvfrom");
        }
        close(sock);
        return -1;
    }
    buf[recvd] = '\0';

    close(sock);
    return 0;
}

int lvgl_socket_recv(char *buf, size_t bufsize) {
    return lvgl_socket_recv_timeout(buf, bufsize, LVGL_SOCKET_RECV_TIMEOUT_MS);
}
