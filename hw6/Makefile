all:
	gcc -Wall -Werror -pthread client.c  sfwrite.c -o client -g
	gcc -Wall -Werror -pthread server.c sfwrite.c -o server -g
	gcc -Wall -Werror -pthread chat.c -o chat

clean:
	rm -f *~ *.o  client
	rm -f *~ *.o server
	rm -f *~ *.o chat
