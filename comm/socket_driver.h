#ifndef SOCKET_DRIVER_H
#define SOCKET_DRIVER_H

#include <stddef.h>

#define LVGL_READ_SOCKET_PATH "/tmp/lvgl_read.sock"
#define LVGL_WRITE_SOCKET_PATH "/tmp/lvgl_write.sock"

int lvgl_socket_send(const char *msg);
int lvgl_socket_recv(char *buf, size_t bufsize);

#endif // SOCKET_DRIVER_H
