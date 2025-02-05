{ pkgs ? import <nixpkgs> {} }:
pkgs.mkShell {
  buildInputs = [
    pkgs.gcc         # sometimes needed for libstdc++ headers
    pkgs.cmake
    pkgs.gnumake
    pkgs.glibc.dev   # provides the C library headers
  ];
  # Optionally, explicitly set CPATH if necessary:
  CPATH = "${pkgs.glibc.dev}/include";
}
