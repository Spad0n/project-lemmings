let pkgs = import <nixpkgs> {};
in pkgs.mkShell {
  nativeBuildInputs = with pkgs; [ pkg-config cmake bear ];
  buildInputs = with pkgs; [
    # for raylib
    glfw
    xorg.libXrandr
    xorg.libX11
    xorg.libXinerama
    xorg.libXcursor
    xorg.libXi
    libGL
  ];

  LD_LIBRARY_PATH="$LD_LIBRARY_PATH:${
    with pkgs;
    pkgs.lib.makeLibraryPath [ xorg.libX11 libGL ]
  }";
}
