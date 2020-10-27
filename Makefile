default:	
	gcc src/main.c -o bin/main -Wall -lm -fsanitize=address,leak
