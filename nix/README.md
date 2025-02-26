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

and if you have a `default.nix` file, you can omit the filename.
