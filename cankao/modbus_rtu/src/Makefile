APP = modbus_rtu
BUILD_DIR = ./build
OBJSDIR = ./build/obj
ELF_FILE = $(BUILD_DIR)/$(APP).elf
MAP_FILE = $(BUILD_DIR)/$(APP).map.sym

DB_INCLUDE = -I./
DB_INCLUDE += -I$(STAGING_DIR)/usr/include

DB_CFLAGS += $(DB_INCLUDE) -g
LIB= -L$(STAGING_DIR)/usr/lib -lblobmsg_json -lubox -luci -lubus -ldebug -ljson-c -ldl  -lmodbus -lm -llua  -ldl

VPATH += cfg:ubus:tool .
SRCDIRS += cfg ubus tool .


SRCFILES = $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.c))
SRCFILENAMES = $(notdir $(SRCFILES))
OBJS = $(patsubst %.c, %.o,$(SRCFILENAMES))
$(warning !!!!!!!!!!! )
$(warning $(SRCFILES) )
$(warning $(SRCFILENAMES) )
$(warning $(OBJS) )
$(warning !!!!!!!!!!! )
build: make-dir $(OBJSDIR) $(OBJS) $(TARDIR)
	$(CC) -o $(ELF_FILE) $(OBJSDIR)/*.o $(LINKFLAGS) $(LIB)

$(OBJS): %.o:%.c
	$(CC) -c $(CFLAGS) $(DB_CFLAGS) $< -o $(OBJSDIR)/$@

.PHONY: make-dir
make-dir:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(OBJSDIR)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
