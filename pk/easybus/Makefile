export EASYBUS_ROOT := $(shell pwd)
#export EASYBUS_ROOT := /home/chenxiong/mtk8653/trunk/product/N5580I/code/easybus
TARGET_DIRS := 	easybus \
								easycontrol \
								easycommon
include ./env.mk

all :
	@echo start! $(EASYBUS_ROOT)
	[ -d $(EASYBUS_ROOT)/obj ] || mkdir $(EASYBUS_ROOT)/obj
	#@$(call build_obj) #调用过程
	make -C $(EASYBUS_ROOT)/easycommon
	make -C $(EASYBUS_ROOT)/easycontrol
	make -C $(EASYBUS_ROOT)/easybus
	$(CC) $(CFLAGS) -fPIC -o ./obj/easycontrol ./obj/*.o
	$(STRIP) ./obj/easycontrol
	$(AR) -r ./obj/libeasybus.a ./obj/*.o
	$(CP) -vf ./obj/libeasybus.a $(EASYBUS_ROOT)/../lib/mtk8653
clean :
	$(RM) ./obj/*.o ./out/*.a ./obj/
