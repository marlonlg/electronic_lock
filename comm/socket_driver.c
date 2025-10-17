#include "socket_driver.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int lvgl_socket_send(const char *msg) {
    int sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock == -1) {
        perror("socket");
        return -1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, LVGL_READ_SOCKET_PATH, sizeof(addr.sun_path) - 1);

    ssize_t sent = sendto(sock, msg, strlen(msg), 0,
                          (struct sockaddr*)&addr, sizeof(addr));
    if (sent == -1) {
        perror("sendto");
        close(sock);
        return -1;
    }

    close(sock);
    return 0;
}

int lvgl_socket_recv(char *buf, size_t bufsize) {
    int sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock == -1) {
        perror("socket");
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
        perror("recvfrom");
        close(sock);
        return -1;
    }
    buf[recvd] = '\0';

    close(sock);
    return 0;
}
