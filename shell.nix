{ pkgs ? import <nixpkgs> {} }:
pkgs.mkShell {
  nativeBuildInputs = with pkgs; [ pkg-config cmake bear ];
  buildInputs = with pkgs; [
    # for raylib
    xorg.libXrandr
    xorg.libX11
    xorg.libXinerama
    xorg.libXcursor
    xorg.libXi
    libGL
  ];

  LD_LIBRARY_PATH="${
    with pkgs;
    pkgs.lib.makeLibraryPath [ xorg.libX11 libGL ]
  }:./raylib/lib64:./";
}
