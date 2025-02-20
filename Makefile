# $@: nom de la cible
# $<: nom de la première dépendance
# $^: liste des dépendances
# $?: liste des dépendances plus récentes que la cible
# $*: nom d'un fichier sans son suffixe

export PKG_CONFIG_PATH = ./raylib/lib/pkgconfig:./raylib/lib64/pkgconfig
CFLAGS  := `pkg-config --cflags raylib` -Wall -Wextra -Wno-unused-result -std=gnu99
LDFLAGS := `pkg-config --libs raylib` -lm -lpthread -ldl -lglfw '-Wl,-rpath,./raylib/lib:./raylib/lib64:'

all: main

MAIN_SRCS := src/main.c src/plug.c src/xml.c src/entity.c src/layout.c src/array.c
DEBUG_SRCS := src/plug.c src/entity.c src/layout.c src/array.c src/xml.c

main: $(MAIN_SRCS) raylib
	gcc -g $(CFLAGS) $(MAIN_SRCS) -o $@ $(LDFLAGS)

debug: src/main.c libplug
	gcc -g $(CFLAGS) -DHOTRELOAD src/main.c -o main $(LDFLAGS)

libplug: $(DEBUG_SRCS) raylib-shared
	gcc $(CFLAGS) -fPIC -shared $(DEBUG_SRCS) -o libplug.so $(LDFLAGS)

raylib:
	mkdir -p ./raylib-src/build
	mkdir -p ./raylib
	cmake -B raylib-src/build -DCMAKE_INSTALL_PREFIX=./raylib -DCUSTOMIZE_BUILD=ON -DUSE_EXTERNAL_GLFW=ON raylib-src
	$(MAKE) -j4 install -C ./raylib-src/build
	rm -rf ./raylib-src/build

raylib-shared:
	mkdir -p ./raylib-src/build
	mkdir -p ./raylib
	cmake -B raylib-src/build -DCMAKE_INSTALL_PREFIX=./raylib -DCUSTOMIZE_BUILD=ON -DUSE_EXTERNAL_GLFW=ON -DBUILD_SHARED_LIBS=ON raylib-src
	$(MAKE) -j4 install -C ./raylib-src/build
	rm -rf ./raylib-src/build

clean:
	rm -rf *.o *~ libplug.so main

reset: clean
	rm -rf ./raylib
	rm -rf ./raylib-src/build
