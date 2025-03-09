# Execute with `nix-instantiate --eval --strict file_a.nix`
let
    exp1 = 1 + 2;
    exp2 = rec { 
        one = exp1;
        two = one + 1;
        three = two + 1;
    };
    name = "Nix";

    # The <> syntax returns a file on the nix path - I don't know why the empty attribute set is needed.
    # Note it is discouraged to use the <> lookup syntax.
    pkgs = import <nixpkgs> {};

    # The ... syntax allow attributes not explicity defined in the lambda arguments.
    # The @ syntax gives the attribute list input a name that can be used lin the lambda.
    f = {a, b, ...}@args: a + b + args.c;
in 
{
    # The with expression allows access to the attribute set namespace. You can use it with any expression.
    with_expression = with exp2; [ one two three ];

    # pulls variables from an existing scope into the current one. Wrap it in { } to create an attribute set.
    inherit_expression = { inherit exp1 exp2; };

    # String concatentation.
    string_expresion = "hello ${name}";

    # import evaluates the expression from another file. It can be a directory too if default.nix exists.
    imported_expression = import ./file_b.nix;

    # We call the toUpper function from the pkgs.lib
    using_pkgs_lib = pkgs.lib.strings.toUpper "lookup paths are considered harmful";

    # We can pass an attribute set to a closure like so.
    closure_expression = f { a = 1; b = 2; c = 3; };
}