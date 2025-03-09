# nix

<https://nix.dev/>
<https://nix.dev/manual/nix/2.24/introduction>

## Installation

<https://nixos.org/download/#download-nix>

On windows, you need to use WSL2. Log in and execute the following command to install the single user environment.

```bash
sh <(curl -L https://nixos.org/nix/install) --no-daemon
```

Verif you have nix installed

```bash
nix --version
```

## Ad hoc shell

Create a shell with given packages on the path. Then you can execute the contents of the those pacakges

```bash
nix-shell -p cowsay lolcat
cowsay Hello, Nix! | lolcat
```

Terminate the shell with `exit` or `CTRL-D`.

## Declaritive shell environments

You can create `shell.nix` file that defines the nix shell when launched. This can include the packages to install
and the environmental variables in the shell. See `workspace-a/shell.nix` for an example.

When your present working directory contains a `shell.nix` file, just execute,

```bash
nix-shell
```

## Nix Interactive Shell

Launch an interactive shell to test nix language features.

```bash
nix repl
```

`:q` termiantes the repl. `:p` evaluates and prints the resulting expression. `:?` displays help.

You can execute a nix file with, 

```bash
nix-instantiate --eval file.nix
```

and if you have a `default.nix` file, you can omit the filename. Use the `--strict` option if you want the expression
evaluate rather than returning the unevaluated expression.

## Nix Libraries

There are two main libraries that are shipped with Nix `builtins` and `pkgs.lib` from nixpkgs. Basically are library
is a attribute set with all its elements being various closures that you can execute.

## Impurites, Paths, and Fetchers

We read files from the Nix store as build inputs. These are considered impurities because the outcome of a closure
can change depending on the contents of the build inputs.

If you refer to file in a string interpolation the file is copied to the nix store. 

```nix
nix-repl> "${./data.txt}"
"/nix/store/5jbw52kxdk1jx49h1d7djs6pj391dxng-data.txt"
```

If you change the contents and run a nix application again, it will copy the file to the nix store with a different
hash. This is an impurity because if you change the file you get different results.

There are also builtin function to move content into the nix store from URLs, tarballs, git, and other closure.

```nix
builtins.fetchurl
builtins.fetchTarball
builtins.fetchGit
builtins.fetchClosure
```

And these have some additional optional parameters that you can use. The key is that if the binary of the resource 
changes, these will become a separate nix packages.

## Derivations

Nix runs derivations to produce build results, and build results can be used as inputs to other Nix derivations.

There is a keyword `derivation` to create derivations, but typically you use the `stdenv.mkDerivation` closure
instead. Executing these function results in an attribute with various properties...

QUESTION: What are these properties?

Using string interpolation on derivations outputs its build result in the nix store.

## Remove unused build results

```bash
nix-collect-garbage
```

# A Definition of a Package

A package is a loosely defined concept that refers to either a collection of files and other data, or a Nix expression 
representing such a collection before it comes into being. Packages in Nixpkgs have a conventional structure, allowing 
them to be discovered in searches and composed in environments alongside other packages.

For the purposes of this tutorial, a “package” is a Nix language function that will evaluate to a derivation. It will 
enable you or others to produce an artifact for practical use, as a consequence of having “packaged existing software 
with Nix”.

# TODO

Follow the learn more links here https://nix.dev/tutorials/nix-language#learn-more
The standard env https://nixos.org/manual/nixpkgs/unstable/#part-stdenv