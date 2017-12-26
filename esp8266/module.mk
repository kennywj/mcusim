$(warning  components/esp8266/module.mk)
local_dir	:= components/esp8266
lib_dir		:= $(local_dir)
lib_name	:= #??.a
header_dir	:= $(TOP)/components/esp8266

local_lib	:= #$(lib_dir)/$(lib_name)
# compile source
local_src	:= 

ifeq "y" "$(CONFIG_IOT_ESP8266)"
local_src +=  cobs.c comm.c crc16.c esp8266_if.c netconf.c
endif


lib_src		:=


include build/common.mk
