#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <unistd.h>

#ifndef SOCKET_PATH_STR
#define SOCKET_PATH_STR "/tmp/img-service"
#endif
