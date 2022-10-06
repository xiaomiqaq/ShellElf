CC = aarch64-linux-gnu-gcc #ARM g++
SRC = shell.c
Target = shell.o

shell :shell.o
	$(CC) -o $@ $^ --static
shell.o: $(SRC)
	$(CC)  -c -o $@ $^ --static
clean:
	-rm -f $(Target)