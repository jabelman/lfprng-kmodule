VERSION = 2
       PATCHLEVEL = 6
       SUBLEVEL = 29
       EXTRAVERSION =

       obj-m += fortune.o

       KDIR=/home/josh/AOSP_DIRECTORY/device/generic/goldfish/goldfish
       PWD := $(shell pwd)

       default:
		make -C $(KDIR) SUBDIRS=$(PWD) modules
       clean:
		make -C $(KDIR) SUBDIRS=$(PWD) clean
	   test: test.c
	    gcc -0 test -lpthread test.c