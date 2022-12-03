all:compiler_combiner

compiler_combiner:
	gcc -o combiner combiner.c  -lpigpio -lpthread -pthread -Werror

clean:
	rm -rf *.o combiner
