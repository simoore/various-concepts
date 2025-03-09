# Workspace B

A workspace contains a set of related packages. Our IDE should be setup to work in one of the workspaces.

## Building packages

To build all packages run `nix-build` on this directory. This cause the `default.nix` to be evaluated. If you want
to build only one of the packages from this workspace, notice that `default.nix` returns an attribute set, and if
you only evaluate one of the attributes, only one package gets build. For example, run the following to build the 
gnu-hello package,

```bash
# The attribute "gnu_hello" is return in attribute set indefault.nix 
nix-build -A gnu_hello
```

After the calling `nix-build`, you can see the resulting package in the `result` symbolic link in your working
directory which links to the package you just built. If you build multiple packages you will see `result`, 
`result-2` etc.

## Searching Nixpkgs

<https://nix.dev/tutorials/packaging-existing-software#finding-packages>

Determining from where to source a dependency is currently somewhat involved, because package names don’t always 
correspond to library or program names.

You will need the Xlib.h headers from the X11 C package, the Nixpkgs derivation for which is libX11, available in the 
xorg package set. There are multiple ways to figure this out:

The easiest way to find what you need is on <search.nixos.org/packages>.

Unfortunately in this case, searching for x11 ((https://search.nixos.org/packages?query=x11) produces too many 
irrelevant results because X11 is ubiquitous. On the left side bar there is a list package sets, and selecting xorg 
shows something promising.

In case all else fails, it helps to become familiar with searching the Nixpkgs source code 
(https://github.com/nixos/nixpkgs) for keywords. Follow 
<https://nix.dev/tutorials/packaging-existing-software#local-code-search>

TODO: Try and use <nix-locate> and <nix-index> commands.
