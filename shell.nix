{ pkgs ? import <nixpkgs> {} }:
let
  pkgsOld = import (fetchTarball {
    url = "https://github.com/NixOS/nixpkgs/archive/92d295f588631b0db2da509f381b4fb1e74173c5.tar.gz";
  }) {};
in
pkgs.mkShell {
  nativeBuildInputs = with pkgs; [ pkg-config cmake bear ];
  buildInputs = [
    pkgsOld.glfw
    pkgs.libGL
  ];

  LD_LIBRARY_PATH="${
    with pkgs;
    pkgs.lib.makeLibraryPath [ libGL ]
  }:./raylib/lib64:./";
}
