all:compile_encoder

compile_encoder:
	gcc -o encoder encoder_maximov.c rotary_encoder.c -lpigpio -lpthread -pthread -Werror

clean:
	rm -rf *.o encoder
