let
    # The ... syntax allow attributes not explicity defined in the lambda arguments.
    # The @ syntax gives the attribute list input a name that can be used lin the lambda.
    f = {a, b, ...}@args: a + b + args.c;
in
f { a = 1; b = 2; c = 3; }