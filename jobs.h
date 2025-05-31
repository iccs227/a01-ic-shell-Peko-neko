#ifndef JOBS_H
#define JOBS_H

#include "icsh.h"

int find_job_by_id(int id);
void print_jobs();
void handle_fg(char* arg, int* exit_code);
void handle_bg(char* arg, int* exit_code);

#endif
