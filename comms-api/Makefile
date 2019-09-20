libcomms.a: init.o pubsub.o
	ar rc lib/libcomms.a init.o pubsub.o
	rm init.o
	rm pubsub.o
	gcc test/src/inproctest.c -I./include -L./lib -lcomms -o bin/iptest -lpthread -ljson-c -lzmq
	gcc test/src/inproctest2.c -I./include -L./lib -lcomms -o bin/iptest2 -lpthread -ljson-c -lzmq
	gcc test/src/ipctest1.c -I./include -L./lib -lcomms -o bin/ipctest1 -lpthread -ljson-c -lzmq
	gcc test/src/ipctest2.c -I./include -L./lib -lcomms -o bin/ipctest2 -lpthread -ljson-c -lzmq

init.o:
	gcc -o init.o -c src/init.c -I./include

pubsub.o:
	gcc -o pubsub.o -c src/pubsub.c -I./include
