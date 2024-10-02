#ifndef SERVER_H
#define SERVER_H

#include <stdbool.h>

#include "cli.h"
#include "network.h"

void server_start(const config config);
void server_stop(const config config);

#endif