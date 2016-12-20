include ../../build/build_head.mk
CFLAGS = -D$(CONFIG_PLATFORM) -D$(LOG_LEVEL) -g -Wall -I$(INCLUDE_PATH) -I$(INCLUDE_PATH_PLATFORM) -Wno-unused-function
LDFLAGS = -L$(OUT_LIBS_SHARE)/$(PLATFORM) -L$(LIBS_SHARE_PLATFORM) $(LIBS_PRIV) -lpthread -lsys_utils -lbroadcast_utils -lrc4 -lboard_info_utils -llogd_utils -lplatform_utils -L. -lsona_gui -lsam_conf -ltty_utils
include ../../build/build_executable.mk
