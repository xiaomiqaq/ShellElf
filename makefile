CC = aarch64-linux-gnu-gcc #ARM g++
Target = tests/hello_a64
Target_ = tests/hello_a64_

shell:./stub/shell.c
	$(CC)   -c -o ./stub/shell.o  ./stub/shell.c --static

run: 
	qemu-aarch64-static ./Tests/hello_a64
run_: $(Target_)
	qemu-aarch64-static ./$(Target_)

.PHONY:clean
clean:
	-rm -f $(Target_) shell.o