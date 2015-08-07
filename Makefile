# Building-related stuff
objects = core.o \
	utils/frame-puller.o utils/frame-pusher.o utils/list.o \
	effects/action_instant.o effects/action_interval.o effects/audio.o \
	effects/clips.o effects/drawing.o effects/ease.o effects/transition.o

lib_args = `pkg-config --cflags --libs libavformat libswscale libswresample` `freetype-config --cflags` -lfreetype

libdaku: $(objects)
	ar rcs libdaku.a $(objects)

core.o: core.h utils/frame-pusher.h types.h
utils/frame-puller.o: utils/frame-puller.h
utils/frame-pusher.o: utils/frame-pusher.h
utils/list.o: utils/list.h
effects/action_instant.o: effects/actions.h types.h
effects/action_interval.o: effects/actions.h types.h
effects/audio.o: effects/actions.h types.h
effects/clips.o: effects/actions.h utils/frame-puller.h types.h
	cc -c -o effects/clips.o effects/clips.c $(lib_args)
effects/drawing.o: effects/drawing.h types.h
effects/ease.o: effects/ease.h types.h
effects/transition.o: effects/actions.h types.h

playplay: libdaku demo/playplay.c
	cc -o demo/playplay demo/playplay.c libdaku.a $(lib_args)

.PHONY: clean
clean:
	rm -f effects/*.o
	rm -f utils/*.o
	rm -f *.o
	rm -f libdaku.a
	rm -f demo/playplay

# Installation-related stuff
INSTALL = install -p
INSTALL_EXEC = $(INSTALL) -m 0755
INSTALL_DATA = $(INSTALL) -m 0644
#
# If you don't have "install" you can use "cp" instead.
# INSTALL = cp -p
# INSTALL_EXEC = $(INSTALL)
# INSTALL_DATA = $(INSTALL)

INSTALL_LOC_INCLUDE = /usr/local/include/libdaku
INSTALL_LOC_LIB = /usr/local/lib
INSTALL_DIRS = $(INSTALL_LOC_INCLUDE) $(INSTALL_LOC_LIB) \
	$(INSTALL_LOC_INCLUDE)/effects $(INSTALL_LOC_INCLUDE)/utils

INSTALL_INCLUDE = daku.h effects/actions.h effects/clips.h effects/drawing.h \
	effects/ease.h utils/frame-puller.h utils/frame-pusher.h utils/list.h \
	core.h types.h
INSTALL_LIB = libdaku.a

install:
	mkdir -p $(INSTALL_DIRS)
	$(INSTALL_DATA) $(INSTALL_INCLUDE) $(INSTALL_LOC_INCLUDE)
	$(INSTALL_DATA) $(INSTALL_LIB) $(INSTALL_LOC_LIB)

uninstall:
	rm -rf $(INSTALL_LOC_INCLUDE)
	cd $(INSTALL_LOC_LIB) && rm -f $(INSTALL_LIB)
