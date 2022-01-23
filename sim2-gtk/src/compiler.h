#ifndef _COMPILER_H_
#define _COMPILER_H_

#include <glib.h>
#include "program.h"

gchar *
compiler_get_error_message ();

Program *
compiler_get_program (GFile *file);

#endif /* _COMPILER_H_ */
