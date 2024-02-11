enum LocationStatus {
    PreyHere,
    PredHere,
    PreyRestingHere,
    PredRestingHere,
    NothingHere,
}

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
