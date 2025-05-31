#ifndef SIGNALS_H
#define SIGNALS_H

#include <signal.h>

void sigint_handler(int sig);
void sigtstp_handler(int sig);
void sigchld_handler(int sig);

#endif
