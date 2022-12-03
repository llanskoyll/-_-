#include "combiner.h"

pthread_mutex_t mutex_button_record = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_time = PTHREAD_MUTEX_INITIALIZER;

bool button_record;
time_t _time;

// т.к время может быть не "настоящим", а результатом изменений, то сделаем такой костыль
static void *time_counter() 
{
    int fd;
    int ret;
    char time_to_lcd[10];
 
    while(1) {
        sleep(1);
        pthread_mutex_lock(&mutex_time);
        printf("Текущие время : %ld\n", _time);
        _time++;

        pthread_mutex_unlock(&mutex_time);

        // канал для lcd
        sprintf(time_to_lcd, "%d", _time);

        fd = open("lcd_fifo", O_WRONLY);
        if (fd == -1) {
            printf("Failed open pipe for write\n");
        }

        ret = write(fd, time_to_lcd, sizeof(time_to_lcd));
        if (ret == -1) {
            printf("Failed write to pipe!\n");
        }
        close(fd);
    }
}
// функция обработки кнопок
// текущий gpio кнопки 26
static void *button_handler(void *argv)
{
    while (1) {
        sleep(1);
        FILE *fp = 0;
        int fd;
        char value_;
        int value_button_record;

        fp = fopen("/sys/class/gpio/gpio26/value", "r");

    // чтение значение из файла нажатой кнопки
        fread((void *)&value_, sizeof(value_), sizeof(value_), fp);

        pthread_mutex_lock(&mutex_button_record);

        // кнопка нажата button_record = true
        if (!atoi(&value_)) {
            printf("Кнопка нажата!\n");
            button_record = true;    
        } else {
            printf("Кнопка не нажата!\n");
            button_record = false;
        }

        pthread_mutex_unlock(&mutex_button_record);

        fclose(fp);
    }
}

static void time_print(int time_sec) 
{
    usleep(500);
    pthread_mutex_lock(&mutex_time);

    _time = _time + time_sec;
    printf("Время изменено на %ld!\n", _time);

    pthread_mutex_unlock(&mutex_time);
}

// функция обработки именнованнымии каналов
static void *pipe_handler(void *argv)
{
    mkfifo("lcd_fifo", 0666);
    int fd;
    char time_to_encoder[5];
    int ret;
    int value_from_pipe;
 
    while(1) {
        
        char *path = (char *)argv;
        fd = 0;


        // чтение из encoder fifo
        fd = open(path, O_RDONLY);
        if (fd == -1) {
            printf("Failed open to pipe!\n");
        }

        read(fd, (void *)time_to_encoder, sizeof(value_from_pipe));
        close(fd);

        value_from_pipe = (atoi(time_to_encoder))/18;

        pthread_mutex_lock(&mutex_button_record);
        
        if (button_record) {
            printf("Сигнал изменение времени на %d секунд\n", value_from_pipe);
            time_print(value_from_pipe);
        }

        pthread_mutex_unlock(&mutex_button_record);

    }
}

int main(int argc, char **argv) 
{
    // argv[1] - путь до именнованого канала
    if (argc != 2) {
        printf("Uncorrect count argument !\n");
        return 0;
    }

    button_record = false;
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