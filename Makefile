obj-m += proj2.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules > /dev/null

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

run:
	sudo dmesg --clear && sudo insmod ./proj2.ko int_str='"-1 0 1 2 33"' && sudo rmmod proj2 && dmesg

clear:
	clear

buildrun: clear all run
