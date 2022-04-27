server: sdstored

client: sdstore

sdstored: obj/sdstored.o obj/filaEspera.o obj/pedido.o
	gcc -g obj/sdstored.o -o sdstored

obj/sdstored.o: src/sdstored.c libs/sdstored.h
	gcc -g -o obj/sdstored.o src/sdstored.c -c -Wall

obj/filaEspera.o: src/filaEspera.c libs/filaEspera.h
	gcc -g -o obj/filaEspera.o src/filaEspera.c -c -Wall

sdstore: obj/sdstore.o
	gcc -g obj/sdstore.o -o sdstore

obj/sdstore.o: src/sdstore.c libs/sdstore.h
	gcc -g -o obj/sdstore.o src/sdstore.c -c -Wall

clean:
	rm -f obj/* tmp/* sdstore sdstored outputs/* server-client client-server