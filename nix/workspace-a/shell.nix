let
    # Assignments are proceeded by a semicolon. fetchTarball is a function amd the string is its first parameter.
    nixpkgs = fetchTarball "https://github.com/NixOS/nixpkgs/tarball/nixos-24.05";

    # The {} syntax is an attribute set (ie. map or dict). The [] syntax is a list.
    pkgs = import nixpkgs { config = {}; overlays = []; };

in 
pkgs.mkShellNoCC {

    # These are the packages installed.
    packages = with pkgs; [
        cowsay
        lolcat
    ];

    # This becomes an environmental variable. This works as any attribute passed to mkShellNoCC that cannot be resolved
    # is interpreted as an environmental variable. 
    GREETING = "Hello, Nix!";

    # This is a starup command. Make sure you line endings use only \n when using the '' ''  syntax.
    shellHook = ''
    echo $GREETING | cowsay | lolcat
    '';
}
