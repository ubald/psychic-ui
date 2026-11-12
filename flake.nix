{
  description = "Psychic UI: the toolchain and system libraries for its CMake build";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  # The last release with Python 2, which Skia's 2018 build scripts need.
  inputs.nixpkgs-python2.url = "github:NixOS/nixpkgs/nixos-25.11";

  outputs =
    { nixpkgs, nixpkgs-python2, ... }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };
      python2 =
        (import nixpkgs-python2 {
          inherit system;
          config.permittedInsecurePackages = [ "python-2.7.18.12" ];
        }).python27;
    in
    {
      # Skia, Yoga and depot_tools still come from the pinned submodules; this shell
      # supplies what their builds expect to find on the system, and the packages
      # CMake's find_package finds before it would fetch them (SDL3, GLFW, Catch2).
      devShells.${system}.default = pkgs.mkShell {
        name = "psychic-ui";

        packages = with pkgs; [
          cmake
          ninja
          pkg-config
          git
          python2
          doxygen

          libGL
          libpng
          libjpeg_turbo
          freetype
          fontconfig
          icu
          sdl3
          glfw
          zlib
          expat

          # Found by the tests' FetchContent instead of downloading it
          catch2_3

          # X11 development files: Skia's GLX code, and SDL3 or GLFW when they're fetched
          libx11
          libxrandr
          libxinerama
          libxcursor
          libxi
          libxext
          libxxf86vm
        ];
      };
    };
}
