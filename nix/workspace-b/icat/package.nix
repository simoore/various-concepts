let
    version = "v0.5";
in
{
    stdenv,
    fetchFromGitHub,

     # This dependency is in nixpkgs: https://search.nixos.org/packages?query=imlib2
    imlib2,           
    xorg,
}:

stdenv.mkDerivation {
    pname = "icat";
    version = version;
    src = fetchFromGitHub {
        owner = "atextor";
        repo = "icat";
        rev = version;
        sha256 = "0wyy2ksxp95vnh71ybj1bbmqd5ggp13x3mk37pzr99ljs9awy8ka";
    };
    buildInputs = [ imlib2 xorg.libX11 ];

    # Since there is no 'make install' target, you need to define how to store the build artifacts in the 
    # output directory (use $out to refer to the output directory). We add in the hooks to give the ability to 
    # insert functionality if we need.
    installPhase = ''
        runHook preInstall
        mkdir -p $out/bin
        cp icat $out/bin
        runHook postInstall
    '';
}
