#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pigpio.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>

static void *button_handler(void *argv);
static void *pipe_handler(void *argv);
static void *time_counter();
static void *thread_console();

static void time_print(int time_sec);

