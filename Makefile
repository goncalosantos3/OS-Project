server: sdstored

client: sdstore

sdstored: obj/sdstored.o obj/emEspera.o obj/emExecucao.o obj/pedido.o obj/funcsAux.o
	gcc -g obj/sdstored.o obj/emEspera.o obj/emExecucao.o obj/pedido.o obj/funcsAux.o -o sdstored

obj/sdstored.o: src/sdstored.c libs/sdstored.h
	gcc -g -o obj/sdstored.o src/sdstored.c -c -Wall

obj/emEspera.o: src/emEspera.c libs/emEspera.h
	gcc -g -o obj/emEspera.o src/emEspera.c -c -Wall

obj/emExecucao.o: src/emExecucao.c libs/emExecucao.h 
	gcc -g -o obj/emExecucao.o src/emExecucao.c -c -Wall

obj/pedido.o: src/pedido.c libs/pedido.h 
	gcc -g -o obj/pedido.o src/pedido.c -c -Wall

obj/funcsAux.o: src/funcsAux.c libs/funcsAux.h
	gcc -g -o obj/funcsAux.o src/funcsAux.c -c -Wall

sdstore: obj/sdstore.o
	gcc -g obj/sdstore.o -o sdstore

obj/sdstore.o: src/sdstore.c libs/sdstore.h
	gcc -g -o obj/sdstore.o src/sdstore.c -c -Wall

clean:
	rm -f obj/* tmp/* sdstore sdstored outputs/* clients-to-server fifo-*