.PHONY : server client
all: server client

server:
	@echo "Compiling server..."
	@gcc -o server src/srv/server.c -lpthread
	@echo "Done"

client:
	@echo "Compiling client..."
	@gcc -o client src/cli/client.c -lpthread
	@echo "Done"

run_server:
	@echo "Starting server..."
	@./server

run_client:
	@echo "Starting client..."
	@./client

clean:	
	@echo "Housekeeping..."
	@rm -f server
	@rm -f client
	@echo "Done"
