# Execute with `nix-instantiate --eval --strict file_a.nix`
let
    exp1 = 1 + 2;
    exp2 = rec { 
        one = exp1;
        two = one + 1;
        three = two + 1;
    };
    name = "Nix";
in 
{
    # The with expression allows access to the attribute set namespace. You can use it with any expression.
    with_expression = with exp2; [ one two three ];

    # inherit creates and attribute set from a variables.
    inherit_expression = { inherit exp1 exp2; };

    # String concatentation.
    string_expresion = "hello ${name}";

    # import evaluates the expression from another file. It can be a directory too if default.nix exists.
    imported_expression = import ./file_b.nix;
}