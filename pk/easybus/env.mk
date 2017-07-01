SUFFIX  = $(CROSS_COMPILE)
CPP     = $(SUFFIX)g++
CC      = $(SUFFIX)gcc
CXX     = $(SUFFIX)g++
LD      = $(SUFFIX)ld
AS      = $(SUFFIX)as
AR      = $(SUFFIX)ar
NM      = $(SUFFIX)nm
STRIP   = $(SUFFIX)strip
OBJCOPY = $(SUFFIX)objcopy
OBJDUMP = $(SUFFIX)objdump
RANLIB  = $(SUFFIX)ranlib

CP      = cp -f
RM      = rm -rf
SORT    = sort
SED     = sed
TOUCH   = touch

#头文件目录 easybus  easycommon  easycontrol
INCLUDE_DIR := -I$(EASYBUS_ROOT)/easybus \
	-I$(EASYBUS_ROOT)/easycommon \
	-I$(EASYBUS_ROOT)/easycontrol

CFLAGS := $(INCLUDE_DIR) -lpthread
LDFLAGS := -lpthread
#对所有的.o文件以.c文件创建它
%.o : %.c
	${CC} ${CFLAGS} -c $< -o $(EASYBUS_ROOT)/obj/$@
