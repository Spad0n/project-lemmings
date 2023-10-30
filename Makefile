export PKG_CONFIG_PATH = ./raylib/lib64/pkgconfig
CFLAGS  := `pkg-config --cflags raylib`
LDFLAGS := `pkg-config --libs raylib` -lm -lglfw

main: raylib
	gcc -g $(CFLAGS) -o main main.c $(LDFLAGS)

raylib:
	mkdir -p ./raylib-src/build
	mkdir -p ./raylib
	cmake -B raylib-src/build -DCMAKE_INSTALL_PREFIX=./raylib raylib-src
	$(MAKE) -j4 install -C ./raylib-src/build
	rm -rf ./raylib-src/build

clean:
	rm -rf *.o *~

rmproper: clean
	rm -rf main

reset: rmproper
	rm -rf ./raylib
	rm -rf ./raylib-src/build
