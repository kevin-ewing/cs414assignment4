
all: fs-find fs-cat

fs-find: fs-find.c
	gcc -g -o fs-find fs-find.c

fs-cat: fs-cat.c
	gcc -g -o fs-cat fs-cat.c

clean: .PHONY
	rm -f *.o fs-find fs-cat