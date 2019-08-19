#ifndef __POSIX_SOCKET_TEMPLATE_H__
#define __POSIX_SOCKET_TEMPLATE_H__

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>

#ifdef __cplusplus
extern "C" {
#endif
/*
    A template for opening a non-blocking POSIX socket.
*/
int open_nb_socket(const char* addr, const char* port);
#ifdef __cplusplus
}
#endif

#endif