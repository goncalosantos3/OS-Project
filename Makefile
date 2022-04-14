server: sdstored

client: sdstore

sdstored: obj/sdstored.o
	gcc -g obj/sdstored.o -o sdstored

obj/sdstored.o: src/sdstored.c
	gcc -g -o obj/sdstored.o src/sdstored.c -c -Wall

sdstore: obj/sdstore.o
	gcc -g obj/sdstore.o -o sdstore

obj/sdstore.o: src/sdstore.c
	gcc -g -o obj/sdstore.o src/sdstore.c -c -Wall

clean:
	rm -f obj/* tmp/* sdstore sdstored