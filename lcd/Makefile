all: compile_lcd

compile_lcd:
	gcc -o lcd main.c lcd.c -lgd -lfreetype -lm -lpthread -pthread -Werror

clean:
	rm -rf *.o lcd
