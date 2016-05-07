obj-m += char_deviceDriver.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	$(CC) test.c -o test
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
