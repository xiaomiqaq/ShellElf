CC = aarch64-linux-gnu-gcc #ARM g++
SRC = hello_a64.c
Target = hello_a64
Target_ = hello_a64_

hello_a64: $(SRC) 
	$(CC) -o $@ $^ --static

run : hello_a64
	qemu-aarch64-static ./$(Target)
run_ : hello_a64_
	qemu-aarch64-static ./$(Target_)

.PHONY:clean
clean:
	-rm -f hello_a64