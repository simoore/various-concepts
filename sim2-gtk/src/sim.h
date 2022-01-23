#ifndef _SIM_H_
#define _SIM_H_

#include <glib.h>
#include "program.h"

typedef enum {PREY_HERE, PRED_HERE, PREY_RESTING_HERE, PRED_RESTING_HERE,
    NOTHING_HERE} LocationStatus;

#define SIZE  100

void 
sim_iteration ();

LocationStatus 
sim_get_location_status (int x, int y);

gchar *
sim_get_error_message ();

void
sim_check ();

void
simulation_free ();

void 
sim_reset (Program *prey, Program *pred, int prey_num, int pred_num);


#endif /* _SIM_H_ */
