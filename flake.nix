{
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
  inputs.flake-utils.url = "github:numtide/flake-utils";

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        shortRev = with self; if sourceInfo?dirtyShortRev then sourceInfo.dirtyShortRev else sourceInfo.shortRev;
        pkgs = (import nixpkgs { inherit system; });
        tools = with pkgs; [
          cmake
          ninja
          pkg-config
        ];
        libs = with pkgs; [
          libchardet
        ];
      in
      {
        packages.default = pkgs.stdenv.mkDerivation
          {
            pname = "detectcharset";
            version = "0.0.1-${shortRev}";
            src = pkgs.lib.fileset.toSource {
              root = ./.;
              fileset = pkgs.lib.fileset.unions [
                ./main.cpp
                ./cflags.h
                ./CMakeLists.txt
              ];
            };
            buildInputs = libs;
            nativeBuildInputs = tools;
            passthru.exePath = "/bin/detectcharset";
          };
        apps.default = {
          type = "app";
          program = "${self.packages.${system}.default}/bin/detectcharset";
        };
        devShells.default = pkgs.mkShell {
          buildInputs = tools ++ libs;
        };
      }
    );
}

