#include "combiner.h"

pthread_mutex_t mutex_button_record = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_time = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_signal_exit = PTHREAD_MUTEX_INITIALIZER;

bool button_record;
bool signal_exit;

time_t _time;

static void *thread_console_check()
{
    char signalch_exit = '1';
    int command = 0;
    int fd;
    int ret;

    pid_t pid_encoder;
    pid_t pid_display;

    while (1) {
        sleep(1);
        printf("Stop display - 1\n");
        printf("Stop encoder - 2\n");
        printf("Stop all - 3\n");
        printf("Run display - 4\n");
        printf("Run encoder - 5\n");
        printf("Set time - 6\n");
        printf("Print information - 7\n");
        printf("Введите команду : ");
        scanf("%d", &command);

        switch(command) {
            case 1:
                // передача в signal_lcd сигнал окончания программы
                fd = open("signal_lcd", O_WRONLY);
                if (fd == -1) {
                    printf("Failed open to signal_lcd !\n");
                }

                ret = write(fd, (void *)&signalch_exit, sizeof(signalch_exit));
                printf("Stop display! \n");
                close(fd);
                break;
            case 2:
                // передача в signal_encoder сигнал окончания программы
                fd = open("signal_encoder", O_WRONLY);
                if (fd == -1) {
                    printf("Failed open to signal_encoder", O_WRONLY);
                }
                write(fd, (void *)&signalch_exit, sizeof(signalch_exit));
                printf("Stop encoder! \n");
                close(fd);
                break;
            case 3:
                fd = open("signal_lcd", O_WRONLY);
                if (fd == -1) {
                    printf("Failed open to signal_lcd !\n");
                }

                ret = write(fd, (void *)&signalch_exit, sizeof(signalch_exit));
                printf("Stop display! \n");
                close(fd);

                fd = open("signal_encoder", O_WRONLY);
                if (fd == -1) {
                    printf("Failed open to signal_encoder", O_WRONLY);
                }
                write(fd, (void *)&signalch_exit, sizeof(signalch_exit));
                printf("Stop encoder! \n");
                exit(0);
                close(fd);
                break;
            case 4:
            // нужен фикс для всех запусков
                pid_display = fork();
                //передать в аргумент путь до lcd_fifo
                static char *argv_display[] = {"lcd", "/home/pi/IVT_32_Maximov/cursach/combiner/lcd_fifo"};
                if (!pid_display) {
                    execv("../lcd/lcd", argv_display);
                }
                break;
            case 5:
                pid_encoder = fork();
                static char *argv_encoder[] = {"/home/pi/IVT_32_Maximov/cursach/encoder/encoder_fifo", "-q"};
                if (!pid_encoder) {
                    execv("../encoder/encoder", argv_encoder);
                }
                break;
            case 6:
                printf("Установить время : ");
                pthread_mutex_lock(&mutex_time);
                scanf("%d", &_time);
                printf("Установленно новое время %ld \n", _time);
                pthread_mutex_unlock(&mutex_time);
                break;
            case 7:
                printf("Время : %ld\n", _time);
                break;

        }
    }
}

// т.к время может быть не "настоящим", а результатом изменений, то сделаем такой костыль
static void *time_counter() 
{
    int fd;
    int ret;
    char time_to_lcd[10];

    while(1) {
        sleep(1);
        pthread_mutex_lock(&mutex_time);
        // printf("Текущие время : %ld\n", _time);
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
            printf("Failed write to pipe time_to_lcd!\n");
        }
        close(fd);
    }
}

// функция обработки кнопок
// gpio кнопка будет с encoder GPIO26
static void *button_handler(void *argv)
{
    while (1) {
        sleep(1);
        FILE *fp = 0;
        int fd;
        char value_;
        int value_button_record;

        fp = fopen("/sys/module/parameters/value", "r");

    // чтение значение из файла нажатой кнопки
        fread((void *)&value_, sizeof(value_), sizeof(value_), fp);

        pthread_mutex_lock(&mutex_button_record);

        // кнопка нажата button_record = true
        if (!atoi(&value_)) {
            // printf("Кнопка нажата!\n");
            button_record = true;    
        } else {
            // printf("Кнопка не нажата!\n");
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
    // printf("Время изменено на %ld!\n", _time);

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
            // printf("Сигнал изменение времени на %d секунд\n", value_from_pipe);
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
    
    mkfifo("signal_lcd", 0666);
    mkfifo("signal_encoder", 0666);

    signal_exit = false;
    button_record = false;
    _time = time(NULL);
    
    pthread_t thread_button_handler; // поток для обрботки кнопок
    pthread_t thread_pipe_handler;
    pthread_t thread_time;
    pthread_t thread_console; // поток для считывания с консоли

    pthread_create(&thread_button_handler, NULL, button_handler, NULL);
    pthread_create(&thread_pipe_handler, NULL, pipe_handler, (void *)argv[1]);
    pthread_create(&thread_console, NULL, thread_console_check, NULL);
    pthread_create(&thread_time, NULL, time_counter, NULL);
    pthread_join(thread_console, NULL);

    if (signal_exit) {
        printf("Program close!\n");
        exit(0);
    }
    pthread_join(thread_button_handler, NULL);
    pthread_join(thread_pipe_handler, NULL);
    pthread_join(thread_time, NULL);
    
    return 0;
}