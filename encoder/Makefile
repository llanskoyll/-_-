all:compile_encoder

compile_encoder:
	gcc -o encoder encoder_maximov.c rotary_encoder.c -lpigpio 

clean:
	rm -rf *.o encoder
