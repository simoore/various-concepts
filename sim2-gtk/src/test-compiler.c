#include <gio/gio.h>
#include "lexer.h"
#include "compiler.h"
#include "program.h"

int
main ()
{
    GFile *file;
    char *path;
    Token token;
    Program *program;

    token = NULLT;
    path = "/doc/Dropbox/sim2-gtk/first.txt";

    file = g_file_new_for_path (path);
    if (file != NULL) 
    {
        lexer_load_file (file);
        while (token != EOFT)
        {
            token = lexer_get_token ();
            g_print ("%d %s\n", token, lexer_get_string_value ());
        }
    } 
    else
        g_print ("file is null\n");

    g_print ("/////////////////////// Compiler Test ///////////////////////\n");

    program = compiler_get_program (file);

    g_print ("///////////////////// Print Program /////////////////////////\n");
    program_print (program);

    program_free (program);
    return 0;
}
