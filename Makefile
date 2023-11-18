# $@: nom de la cible
# $<: nom de la première dépendance
# $^: liste des dépendances
# $?: liste des dépendances plus récentes que la cible
# $*: nom d'un fichier sans son suffixe

export PKG_CONFIG_PATH = ./raylib/lib/pkgconfig:./glfw3/lib/pkgconfig:./raylib/lib64/pkgconfig:./glfw3/lib64/pkgconfig
CFLAGS  := `pkg-config --cflags raylib`
LDFLAGS := `pkg-config --libs raylib glfw3` -lm -lpthread -ldl

all: raylib glfw 2d_camera_platformer chase_in_space

main: src/main.c src/plug.c
	gcc -g $(CFLAGS) $^ -o $@ $(LDFLAGS)

debug: src/main.c libplug.so
	gcc -g $(CFLAGS) -DHOTRELOAD $< -o main $(LDFLAGS)

libplug.so: src/plug.c
	gcc $(CFLAGS) -fPIC -shared $< -o $@ $(LDFLAGS)

2d_camera_platformer: src/2d_camera_platformer.c
	gcc -g $(CFLAGS) $< -o $@ $(LDFLAGS)

chase_in_space: src/chase_in_space.c
	gcc -g $(CFLAGS) $< -o $@ $(LDFLAGS)

raylib:
	mkdir -p ./raylib-src/build
	mkdir -p ./raylib
	cmake -B raylib-src/build -DCMAKE_INSTALL_PREFIX=./raylib raylib-src
	$(MAKE) -j4 install -C ./raylib-src/build
	rm -rf ./raylib-src/build

raylib-shared:
	mkdir -p ./raylib-src/build
	mkdir -p ./raylib
	cmake -B raylib-src/build -DCMAKE_INSTALL_PREFIX=./raylib -DBUILD_SHARED_LIBS=ON raylib-src
	$(MAKE) -j4 install -C ./raylib-src/build
	rm -rf ./raylib-src/build

glfw:
	mkdir -p ./glfw3-src/build
	mkdir -p ./glfw3
	cmake -B glfw3-src/build -DCMAKE_INSTALL_PREFIX=./glfw3 glfw3-src
	$(MAKE) -j4 install -C ./glfw3-src/build
	rm -rf ./glfw3-src/build


clean:
	rm -rf *.o *~

rmproper: clean
	rm -rf 2d_camera_platformer
	rm -rf chase_in_space
	rm -rf main

reset: rmproper
	rm -rf ./raylib
	rm -rf ./glfw3
	rm -rf ./raylib-src/build
	rm -rf ./glfw3-src/build
