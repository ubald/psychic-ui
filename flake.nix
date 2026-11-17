{
  description = "Psychic UI: the toolchain and system libraries for its CMake build";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs =
    { nixpkgs, ... }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };
    in
    {
      # Skia and Yoga still come from the pinned submodules; this shell supplies
      # what their builds expect to find on the system, and the packages CMake's
      # find_package finds before it would fetch them (SDL3, GLFW, Catch2).
      devShells.${system}.default = pkgs.mkShell {
        name = "psychic-ui";

        packages = with pkgs; [
          cmake
          ninja
          python3
          pkg-config
          git
          doxygen

          libGL
          freetype
          fontconfig
          harfbuzz
          icu
          sdl3
          glfw

          catch2_3

          # X11 development files, for SDL3 or GLFW when they're fetched
          libx11
          libxrandr
          libxinerama
          libxcursor
          libxi
          libxext
          libxxf86vm
        ];

        # Under WSL, Nix's Mesa only reaches the GPU through WSL's D3D12 libraries; otherwise GL is llvmpipe
        shellHook = ''
          if [ -d /usr/lib/wsl/lib ]; then
            export LD_LIBRARY_PATH="/usr/lib/wsl/lib''${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
            export GALLIUM_DRIVER=d3d12
          fi
        '';
      };
    };
}
