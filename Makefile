obj-m := test0.o
gpiomodule-objs := gpiomodule_source.o ../source/gpio.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean