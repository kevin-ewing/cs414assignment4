fs-find: fs-find.c
	gcc -g -o fs-find fs-find.c

clean: .PHONY
	rm -f *.o fs-find