#include <gio/gio.h>
#include "lexer.h"

main ()
{
    GFile *file;
    char *path;
    Token token = NULLT;
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
}
