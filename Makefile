# $@: nom de la cible
# $<: nom de la première dépendance
# $^: liste des dépendances
# $?: liste des dépendances plus récentes que la cible
# $*: nom d'un fichier sans son suffixe

export PKG_CONFIG_PATH = ./raylib/lib64/pkgconfig
CFLAGS  := `pkg-config --cflags raylib`
LDFLAGS := `pkg-config --libs raylib` -lm -lglfw

all: 2d_camera_platformer chase_in_space raylib

2d_camera_platformer: src/main.c
	gcc -g $(CFLAGS) $< -o $@ $(LDFLAGS)

chase_in_space: src/chase_in_space.c
	gcc -g $(CFLAGS) $< -o $@ $(LDFLAGS)

raylib:
	mkdir -p ./raylib-src/build
	mkdir -p ./raylib
	cmake -B raylib-src/build -DCMAKE_INSTALL_PREFIX=./raylib raylib-src
	$(MAKE) -j4 install -C ./raylib-src/build
	rm -rf ./raylib-src/build

clean:
	rm -rf *.o *~

rmproper: clean
	rm -rf 2d_camera_platformer
	rm -rf chase_in_space

reset: rmproper
	rm -rf ./raylib
	rm -rf ./raylib-src/build
