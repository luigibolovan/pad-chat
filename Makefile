.PHONY : server client
all: server client

server:
	@echo "Compiling server..."
	@gcc -o server src/srv/server.c src/auth/SearchUser.c src/sockio/send.c  -lpthread
	@echo "Done"

client:
	@echo "Compiling client..."
	@gcc -o client src/cli/client.c src/sockio/send.c -lpthread
	@echo "Done"

clean:	
	@echo "Housekeeping..."
	@rm -f server
	@rm -f client
	@echo "Done"
