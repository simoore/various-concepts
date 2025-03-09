{
    writeShellScriptBin,
    
    # This parameter (which has a default value) can be configured for a different package.
    audience ? "world",
}:

# The package name and script name becomes bash-hello
writeShellScriptBin "bash-hello" ''
    echo "Hello, ${audience}!"
''