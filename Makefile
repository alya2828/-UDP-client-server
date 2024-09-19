all:
	gcc -Wall -Werror -Wextra server.c  -o server
	gcc -Wall -Werror -Wextra client.c -o client
clean:
	rm -f server client