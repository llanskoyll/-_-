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

// функция обработки кнопок
void *button_handler(void *argv)
{

}

// функция обработки именнованнымии каналов
void *pipe_handler(void *argv)
{
    while(1) {
        int fd;
        char str[5];
        int ret;
        int value_from_pipe;

        char *path = (char *)argv;
        printf("%s\n", path);
        fd = open(path, O_RDONLY);
        if (fd == -1) {
            printf("Failed open to pipe!\n");
        }

        read(fd, (void *)str, sizeof(value_from_pipe));
        close(fd);
        value_from_pipe = atoi(str);
        printf("%d\n", value_from_pipe);
        mkfifo("lcd_fifo", 0666);
        sprintf(str, "%d", value_from_pipe);
        fd = 0;
        fd = open("lcd_fifo", O_WRONLY);
        if (fd == -1) {
            printf("Failed open pipe for write\n");
        }
        ret = write(fd, str, 5);
        if (ret == -1) {
            printf("Failed write to pipe!\n");
        }

        close(fd);
    }
}

int main(int argc, char **argv) 
{
    // argv[1] - путь до именнованого канала
    if (argc != 2) {
        printf("Uncorrect count argument !\n");
        return 0;
    }
    //char path[50];
    //strncpy(&path[0], &argv[2], sizeof(path)); 
    pthread_t thread_button_handler; // поток для обрботки кнопок
    pthread_t thread_pipe_handler;

    // запуск поток обработчика кнопок
    pthread_create(&thread_button_handler, NULL, button_handler, NULL);
    pthread_create(&thread_pipe_handler, NULL, pipe_handler, (void *)argv[1]);
    pthread_join(thread_button_handler, NULL);
    pthread_join(thread_pipe_handler, NULL);

    return 0;
}