#include "lcd.h"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>
#include "gdfontg.h"

#define FONT_SIZE_X 9
#define FONT_SIZE_Y 15
#define STRING_MARGIN 5
#define STRING_LENGTH (int)(LCD_WIDTH / FONT_SIZE_X - 1)

pthread_mutex_t mutex_signal_exit = PTHREAD_MUTEX_INITIALIZER;
bool signal_exit;

void *signal_catch()
{
        char signalch;
        int signaldec = 0;
        int fd;
        fd = open("../combiner/signal_lcd", O_RDONLY | O_NONBLOCK);

    while (1) {
        sleep(1);
        // чтение из fifo
        if (fd == -1) {
            printf("Failed open to signal_lcd !\n");
        }
        read(fd, (void *)&signalch, sizeof(signalch));
        signaldec = atoi(&signalch);
        pthread_mutex_lock(&mutex_signal_exit);
            if (signaldec) {
                signal_exit = true;
                printf("Signal Exit\n");
                printf("Stop display!\n");
                close(fd);
                exit(0);
            }
        pthread_mutex_unlock(&mutex_signal_exit);


    }
}

// Demo text
void demo_text(char* text) {

    gdImagePtr im = gdImageCreateTrueColor(LCD_WIDTH, LCD_HEIGHT);
    int white = gdImageColorAllocate(im, 255, 255, 255);
    char buff[STRING_LENGTH + 1];
    int str_ptr = 0;
    int num_str = 0;
    for(int i = 0; i < strlen(text); i++) {
        if(text[i] == '\n' || str_ptr == STRING_LENGTH || text[i+1] == '\0') {
            strncpy(buff, text + i - str_ptr, (text[i] == '\n') ? str_ptr : str_ptr + 1);
            buff[str_ptr + ((text[i] == '\n') ? 0 : 1)] = '\0';
            gdImageString(im, gdFontGetGiant(), 0, num_str * FONT_SIZE_Y + STRING_MARGIN, buff, white);
            str_ptr = 0;
            num_str++;
        }
        else str_ptr++;
    }

    display_gd_image(im);
    gdImageDestroy(im);
}

void help(char* prog) {
    printf("Usage: sudo %s <text>\n", prog);
}

int main(int argc, char* argv[]) {

    lcd_init();
    int fd;
    char str[10];
    bool signal_exit = false;
    
    pthread_t thread_signal;

    pthread_create(&thread_signal, NULL, signal_catch, NULL);

    pthread_detach(thread_signal);
while (1) {
    sleep(1);
    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        printf("Failed to open\r\n");
    }
    read(fd, str, 10);
    // printf("%s\n", str);
    demo_text(str);
    close(fd);
}
    lcd_deinit();
    return EXIT_SUCCESS;
}
