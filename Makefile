# change application name here (executable output name)
TARGET=gradovi-srbije

# compiler
CC=gcc
# warnings
WARN=-Wall -Wextra -Wpedantic

PTHREAD=-pthread

CCFLAGS=$(WARN) $(PTHREAD) -pipe -std=c99
ifdef DEBUG
	CCFLAGS+=-g -O0
endif

GTKLIB=`pkg-config --cflags --libs gtk+-3.0 glib-2.0 json-glib-1.0`

# linker
LD=gcc

LDFLAGS=$(PTHREAD) $(GTKLIB)
ifdef WINDOWS
	LDFLAGS+=-mwindows -Wl,--export-all-symbols
else
	LDFLAGS+=-rdynamic
endif

OBJS=main.o game_data.o game_logic.o map_point.o city.o resources.o
ifdef WINDOWS
	OBJS+=windows-icon-resource.res windows-info-resource.res
endif

all: $(OBJS)
	$(LD) -o $(TARGET) $(OBJS) $(LDFLAGS)

main.o: src/main.c src/game_data.h src/game_logic.h src/map_point.h src/city.h
	$(CC) -c $(CCFLAGS) src/main.c $(GTKLIB) -o main.o

game_data.o: src/game_data.c src/game_data.h src/city.h
	$(CC) -c $(CCFLAGS) src/game_data.c $(GTKLIB) -o game_data.o

game_logic.o: src/game_logic.c
	$(CC) -c $(CCFLAGS) src/game_logic.c $(GTKLIB) -o game_logic.o

map_point.o: src/map_point.c
	$(CC) -c $(CCFLAGS) src/map_point.c $(GTKLIB) -o map_point.o

city.o: src/city.c src/map_point.h
	$(CC) -c $(CCFLAGS) src/city.c $(GTKLIB) -o city.o

resources.o: src/resources.c src/resources.h resources/gradovi-srbije.gresource.xml
	glib-compile-resources resources/gradovi-srbije.gresource.xml --target=src/resources.c --generate-source
	glib-compile-resources resources/gradovi-srbije.gresource.xml --target=src/resources.h --generate-header
	$(CC) -c $(CCFLAGS) src/resources.c $(GTKLIB) -o resources.o

windows-icon-resource.res: resources/windows-icon-resource.rc
	windres.exe resources/windows-icon-resource.rc -O coff -o windows-icon-resource.res

windows-info-resource.res: resources/windows-info-resource.rc
	windres.exe resources/windows-info-resource.rc -O coff -o windows-info-resource.res

clean:
	rm -f *.o $(TARGET).*
