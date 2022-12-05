/*******************************************************************************
 * Copyright (c) 2022 Sergey Balabaev (sergei.a.balabaev@gmail.com)                     *
 *                                                                             *
 * The MIT License (MIT):                                                      *
 * Permission is hereby granted, free of charge, to any person obtaining a     *
 * copy of this software and associated documentation files (the "Software"),  *
 * to deal in the Software without restriction, including without limitation   *
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,    *
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell   *
 * copies of the Software, and to permit persons to whom the Software is       *
 * furnished to do so, subject to the following conditions:                    *
 * The above copyright notice and this permission notice shall be included     *
 * in all copies or substantial portions of the Software.                      *
 *                                                                             *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    *
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE *
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER      *
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,             *
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR       *
 * OTHER DEALINGS IN THE SOFTWARE.                                             *
 ******************************************************************************/
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

#include "rotary_encoder.h"
#include "encoder_err.h"

#define GPIO_PIN_A 8
#define GPIO_PIN_B 11

spthread_mutext_t mutex_signal_exit = PTHREAD_MUTEX_INITIALIZER;

int quiet = 0;
int pos = 0;
bool signal_exit;

static void *signal_catch()
{
  int fd;
  char signalch[1];
  int signaldec;
  while (1) {
      fd = open("../signal_encoder", O_WRONLY);
      if (fd == -1) {
        printf("Failed open to singal_encoder\n");
      }

      read(fd, (void *)signalch, sizeof(signach));

      signaldec = atoi(signalch);
      pthread_mutex_lock(&mutex_signal_exit);
      if (signaldec) {
          signal_exit = true;
      }
      pthread_mutex_unclock(&mutex_signal_exit);
      if (signal_exit) {
        exit(0);
      } 
  }
}

void help()
{
        printf("    Use this application for reading from encoder\n");
        printf("    execute format: ./encoder [-h][-q] \n");
        printf("    return: increment value, when rotate right\n");
        printf("            decrement value, when rotate left\n");
        printf("    -h - help\n");
        printf("    -q - quiet\n");
}

void callback(int way)
{
  int ret;
  int fd;
  char *encoder_fifo = "encoder_fifo";

  
  pos -= way*360/20;
  char str[5];
  if (!quiet) {
    printf("angle increment: %d\n", pos);
  }

  fd = open(encoder_fifo, O_WRONLY);
  if (fd == -1) {
    putErr(E_OPEN_FAILED);
    goto err_callback;
  }

  sprintf(str,"%d", pos);

  ret = write(fd, str, 5);
  if (ret == -1) {
    printf("Failed write in encoder_fifo value is %d\r\n", pos);
    goto err_callback;
  }

  fflush(stdout);
  close(fd);

err_callback:
  fflush(stdout);
  if (fd != -1) {
    close(fd);
  }
  return;
}

void *print_while(void *args) {
  while(1) {
    sleep(1);
    time_t mytime = time(NULL);
    char *time_str = ctime(&mytime);
    time_str[strlen(time_str) - 1] = '\0';
    printf("Current time: %s\r\n", time_str);
    printf("Current value position: %d\r\n", pos);
  }
}

int main(int argc, char *argv[])
{
  if (argc > 1) {
    if ((strcmp(argv[1], "-h") == 0)) {
        help();
        return 0;
    } else {
      if ((strcmp(argv[1], "-q") == 0)) {
              quiet = 1;
      }
    }
  }
  if (!quiet) {
    printf("\nThe encoder application was started\n\n");
  }

  char *encoder_fifo = "encoder_fifo";
  int ret;
  char str[5];
  int fd;
  bool signal_exit = false;

  Pi_Renc_t *renc;

  pthread_t thread;
  pthread_t thread_signal;

  pthread_create(&thread_signal, NULL, signal_catch, NULL);
  pthread_create(&thread, NULL, print_while, NULL);
  pthread_join(&thread_signal);
  pthread_mutex_lock(&mutex_signal_exit);
  if (signal_exit) {
    exit(0);
  }
  pthread_mutex_unclock(&mutex_signal_exit);

  pthread_join(&thread);

  ret = mkfifo(encoder_fifo, 0666);
  
  if (gpioInitialise() < 0) {
    return 1;
  }

  renc = Pi_Renc(GPIO_PIN_A, GPIO_PIN_B, callback);
  sleep(300);
  Pi_Renc_cancel(renc);
  fflush(stdout);
  gpioTerminate();

}
