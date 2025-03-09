let
    # This fetches the tarball. I think it unpacks it and returns the directory to it.
    # https://nix.dev/manual/nix/2.25/language/builtins.html#builtins-fetchTarball
    nixpkgs = fetchTarball "https://github.com/NixOS/nixpkgs/tarball/nixos-24.05";

    # This reads the default.nix from the unpacked tarball and we evalate it to get the pkgs.
    pkgs = import nixpkgs { config = {}; overlays = []; };
in
rec {
    # We use the callPackage function to build the derivation.
    bash_hello = pkgs.callPackage ./bash-hello/package.nix { audience = "people"; };
    
    gnu_hello = pkgs.callPackage ./gnu-hello/package.nix {};

    # You can make a new package by simply overriding the parametes of an existing one.
    hello_folks = bash_hello.override { audience = "folks"; };

    icat = pkgs.callPackage ./icat/package.nix {};
}
