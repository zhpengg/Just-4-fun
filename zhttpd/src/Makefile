all:
	gcc -o zhttpd server.c parse_args.c parse_request.c readline.c logger.c worker_queue.c waiting_queue.c master.c worker.c file_access.c -Wall -D_GNU_SOURCE

install:
	cp -f zhttpd ../bin/

clean:
	rm zhttpd
