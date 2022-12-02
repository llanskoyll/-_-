#include "combiner.h"

pthread_mutex_t mutex_button_record = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_time = PTHREAD_MUTEX_INITIALIZER;

extern bool button_record = false;
extern time_t _time = 0;

// т.к время может быть не "настоящим", а результатом изменений, то сделаем такой костыль
static void *time_counter(NULL) 
{
    sleep(1);
    phtread_mutex_lock(&mutex_time);

    _time++;

    pthread_mutex_unlock(&mutex_time);
}
// функция обработки кнопок
static void *button_handler(void *argv)
{
    int fd = 0;
    int value_button_record_cur;
    char value_button_record;

// чтение значение из файла нажатой кнопки
    fd = fopen(file_name, "r");

    value_button_record = fgetc(file);
    value_button_record_cur = atoi(value_button_record);

    pthread_mutex_lock(&mutex_button_record);

    // кнопка нажата button_record = true
    if (value_button_record_cur) {
        button_record = true;    
    } else {
        button_record = false;
    }

    pthread_mutex_unlock(&mutex_button_record);
}

static void time_print(int time_sec) 
{
    pthread_mutex_lock(&mutex_time);

    _time = _time - time_sec;
    printf("Время изменено на %ld!\n", _time);

    pthread_mutex_unlock(&mutex_time);
}

// функция обработки именнованнымии каналов
static void *pipe_handler(void *argv)
{
    int fd;
    char str[5];
    int ret;
    int value_from_pipe;
 
    while(1) {
        
        char *path = (char *)argv;
        fd = 0;

        // printf("%s\n", path);

        // чтение из encoder fifo
        fd = open(path, O_RDONLY);
        if (fd == -1) {
            printf("Failed open to pipe!\n");
        }

        read(fd, (void *)str, sizeof(value_from_pipe));
        close(fd);

        value_from_pipe = (atoi(str))/18;

        pthread_mutex_lock(&mutex_button_record);
        
        if (mutex_button_record) {
            printf("Сигнал изменение времени на %d секунд\n", value_from_pipe);
            time_print(value_from_pipe);
        }

        pthread_mutex_unlock(&mutex_button_record);

        // канал для lcd
        mkfifo("lcd_fifo", 0666);
        sprintf(str, "%d", value_from_pipe);

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
    _time = time(NULL);
    
    pthread_t thread_button_handler; // поток для обрботки кнопок
    pthread_t thread_pipe_handler;
    pthread_t thread_time;

    pthread_create(&thread_button_handler, NULL, button_handler, NULL);
    pthread_create(&thread_pipe_handler, NULL, pipe_handler, (void *)argv[1]);
    pthread_create(&thread_time, NULL, time_counter, NULL);

    pthread_join(thread_button_handler, NULL);
    pthread_join(thread_pipe_handler, NULL);
    pthread_join(thread_time, NULL);

    return 0;
}