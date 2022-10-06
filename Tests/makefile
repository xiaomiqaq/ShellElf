CC = aarch64-linux-gnu-gcc #ARM g++
SRC = hello_a64.c
Target = hello_a64

hello_a64: $(SRC)
	$(CC) -o $@ $^ --static

run : hello_a64
	qemu-aarch64-static ./$(Target)
run2 : hello_a64_copy
	chmod 777 hello_a64_copy
	qemu-aarch64-static ./hello_a64_copy
.PHONY:clean
clean:
	-rm -f hello_a64