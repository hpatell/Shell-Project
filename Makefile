CC=gcc
# CC=gcc -Wall

mysh: get_path.o which.o where.o watchuser_list.o linked_list.o  shell-with-builtin.o
	$(CC) -g -pthread shell-with-builtin.c get_path.o which.o where.o watchuser_list.o linked_list.o -o mysh

shell-with-builtin.o: shell-with-builtin.c sh.h
	$(CC) -g -c shell-with-builtin.c

get_path.o: get_path.c get_path.h
	$(CC) -g -c get_path.c

which.o: which.c get_path.h
	$(CC) -g -c which.c

where.o: where.c get_path.h
	$(CC) -g -c where.c

watchuser_list.o: watchuser_list.c watchuser_list.h
	$(CC) -g -c watchuser_list.c

linked_list.0: linked_list.c linked_list.h
	$(CC) -g -c linked_list.c

clean:
	rm -rf shell-with-builtin.o get_path.o which.o where.o watchuser_list.o linked_list.o mysh
