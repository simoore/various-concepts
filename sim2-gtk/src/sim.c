#include <glib.h>
#include "program.h"
#include "sim.h"

typedef enum {PRED, PREY} CreatureType;

typedef struct 
{
    CreatureType type;
    ActionType state;
    Direction direction;
    int energy;
    int x_coord;
    int y_coord;
    int awake_daily;
    int count_down;
    gboolean visible;
} Creature;

typedef struct
{
    int prey_num;
    int pred_num;
    int prey_breed;
    int pred_breed;
    int prey_resting;
    int pred_resting;
    int prey_kill;
} Location;

#define INIT_ENERGY 100

static Location locations[SIZE][SIZE];
static GList *creatures = NULL;
static Program *prey_program = NULL;
static Program *pred_program = NULL;

static int number;

/*******************************************************************************
* Allocates and initialises a new creature object. Be sure to g_free () the 
* structure later.
*
* type      Whether the new creature is predator or a prey.
* x         The x coordinate to place the creature.
* y         The y coordinate to place the creature.
* return    The new creature.
*******************************************************************************/
static Creature *
sim_creature_new (CreatureType type, int x, int y)
{   
    Creature *creature;
    Location *location;
    
    creature = g_new (Creature, 1);
    creature->type = type;
    creature->state = NONE;
    creature->direction = STILL;
    creature->energy = INIT_ENERGY;
    creature->x_coord = x;
    creature->y_coord = y;
    creature->awake_daily = 0;
    creature->count_down = 0;
    creature->visible = TRUE;

    location = &(locations[x][y]);
    if (type == PREY)
    {
        location->prey_num++;
        location->prey_breed++;
    }
    else
    {
        location->pred_num++;
        location->pred_breed++;
    }
    
    return creature;
}

static void
sim_change_creature_energy (Creature *creature, int d_energy)
{
    int original_energy = creature->energy;
    Location *location = &(locations[creature->x_coord][creature->y_coord]);

    creature->energy += d_energy;

    if (original_energy > 25 && creature->energy <= 25)
    {
        if (creature->type == PREY)    
            location->prey_breed--;
        else                        
            location->pred_breed--;        
    }
    if (original_energy <= 25 && creature->energy > 25)
    {
        if (creature->type == PREY)    
            location->prey_breed++;
        else                        
            location->pred_breed++;
    }
}

static void
sim_set_creature_invisible (Creature *creature)
{
    Location *location;
    location = &(locations[creature->x_coord][creature->y_coord]);
    
    if (creature->type == PREY)
    {
        location->prey_num--;
        location->prey_resting++;
        if (creature->energy > 25)    
            location->prey_breed--;
    }
    else
    {
        location->pred_num--;
        location->pred_resting++;
        if (creature->energy > 25)    
            location->pred_breed--;
    }
    creature->visible = FALSE;
}

static void
sim_set_creature_visible (Creature *creature)
{
    Location *location;
    location = &(locations[creature->x_coord][creature->y_coord]);

    if (creature->type == PREY)
    {
        location->prey_num++;
        location->prey_resting--;
        if (creature->energy > 25)    
            location->prey_breed++;
    }
    else
    {
        location->pred_num++;
        location->pred_resting--;
        if (creature->energy > 25)    
            location->pred_breed++;
    }
    creature->visible = TRUE;
}

static void
sim_move (Creature *creature)
{
    Location *location;

    location = &(locations[creature->x_coord][creature->y_coord]);
    if (creature->type == PREY)
    {
        location->prey_num--;
        if (creature->energy > 25) 
            location->prey_breed--;
    }
    else
    {
        location->pred_num--;
        if (creature->energy > 25) 
            location->pred_breed--;
    }
    
    creature->energy--;
    switch (creature->direction)
    {
        case NW: case N: case NE: creature->y_coord--; break;
        case SW: case S: case SE: creature->y_coord++; break;
    }
    switch (creature->direction)
    {
        case NE: case E: case SE: creature->x_coord++; break;
        case NW: case W: case SW: creature->x_coord--; break;
    }
    
    if (creature->x_coord >= SIZE)    
        creature->x_coord = 0;
    else if (creature->x_coord < 0)    
        creature->x_coord = SIZE-1;
    if (creature->y_coord >= SIZE)    
        creature->y_coord = 0;
    else if (creature->y_coord < 0)    
        creature->y_coord = SIZE-1;

    location = &(locations[creature->x_coord][creature->y_coord]);
    if (creature->type == PREY)
    {
        location->prey_num++;
        if (creature->energy > 25) 
            location->prey_breed++;
    }
    else
    {
        location->pred_num++;
        if (creature->energy > 25) 
            location->pred_breed++;
    }
}

static void
sim_rest (Creature *creature)
{
    Location *location;
    location = &(locations[creature->x_coord][creature->y_coord]);
    sim_set_creature_invisible (creature);
    if (creature->count_down < 8)
        creature->energy--;
    else if (creature->count_down >= 24)
        creature->energy = creature->energy - 5;
    creature->awake_daily = 0;
}

static void
sim_hunt (Creature *creature)
{
    Location *location;
    GRand *random;

    sim_move (creature);
    location = &(locations[creature->x_coord][creature->y_coord]);

    if (creature->type == PREY)
        sim_change_creature_energy (creature, 2);
    else if (location->prey_num > 0) // PRED
    {        
        random = g_rand_new ();
        if (creature->energy > g_rand_int_range (random, 0, 100))
        {
            sim_change_creature_energy (creature, 20);
            creature->count_down = 3;
            location->prey_kill++;
        }
        sim_change_creature_energy (creature, -10);
        g_rand_free (random);
    }
}

static void
sim_breed (Creature *creature)
{
    Location *location;
    gboolean is_partner = FALSE;
    Creature *baby;

    sim_move (creature);
    location = &(locations[creature->x_coord][creature->y_coord]);
    if (creature->type == PREY && location->prey_breed > 1) 
        is_partner = TRUE;
    else if (creature->type == PRED && location->pred_breed > 1) 
        is_partner = TRUE;

    if (is_partner == TRUE)
    {
        creature->count_down = 6;
        sim_set_creature_invisible (creature);
        creature->energy = creature->energy - 25;

        // create first baby
        baby = sim_creature_new (creature->type, creature->x_coord, 
            creature->y_coord);
        baby->count_down = 6;
        baby->state = REST;
        sim_set_creature_invisible (baby);
        creatures = g_list_append (creatures, baby);

        // creature second baby
        baby = sim_creature_new (creature->type, creature->x_coord, 
            creature->y_coord);
        baby->count_down = 6;
        baby->state = REST;
        sim_set_creature_invisible (baby);
        creatures = g_list_append (creatures, baby);
    }
    else
        creature->state = NONE;
}

/*******************************************************************************
* Checks if prey is eaten, if so it is purged from the location and its energy
* set to zero. Since it is purged from the location and is dead, no need
* to execute rest of simulation for this creature. The only thing left to
* do is purge the creature from the list.
*******************************************************************************/
static gboolean
sim_is_creature_eaten (Creature *c)
{
    Location *location = &(locations[c->x_coord][c->y_coord]);
    if (c->type == PREY && location->prey_kill > 0 && c->visible == TRUE)
    {
            sim_change_creature_energy (c, -c->energy);
            location->prey_num--;
            location->prey_kill--;
            return TRUE;
    }
    return FALSE;
}

static void
sim_act (gpointer data, gpointer user_data)
{
    Creature *c = data;
    Action *a;
    Location *location;
    number++;

    if (c->count_down > 0)    
        c->count_down--;
    if (c->state != REST)    
        c->awake_daily++;
    if (sim_is_creature_eaten (c) == TRUE) 
        return;

    if (c->count_down == 0)
    {
        // ending long action
        if (c->visible == FALSE)    
            sim_set_creature_visible (c);

        // the creature is thinking
        if (c->type == PREY)
            a = program_execute (prey_program, c->energy, c->awake_daily);
        else if (c->type == PRED)
            a = program_execute (pred_program, c->energy, c->awake_daily);

        // creature has decided nothing
        if (a == NULL)
        {
            c->state = NONE;
            return;
        }

        // creature has decided something
        c->state = a->action;
        if (c->state == REST)
            c->count_down = a->op.time;
        else
            c->direction = a->op.direction;    

        // perform action
        switch (c->state)
        {
            case MOVE:  sim_move (c);  break;
            case REST:  sim_rest (c);  break;
            case HUNT:  sim_hunt (c);  break;
            case BREED: sim_breed (c); break;
        }

        // removes creature if it has died
        if (c->energy <= 0)
        {
            location = &(locations[c->x_coord][c->y_coord]);
            if (c->type == PRED)    
                if (c->visible == TRUE) 
                    location->pred_num--;
                else                    
                    location->pred_resting--;
            else 
                if (c->visible == TRUE) 
                    location->prey_num--;
                else                    
                    location->prey_resting--;
        }
    }    
}

/*******************************************************************************
* This function performs one iteration of the simulation. This involves asking
* each creature their intention which is based on their states and then asking
* each creature to act.
*******************************************************************************/
void 
sim_iteration ()
{
    GList *l = creatures;
    GList *next;
    Creature *c;
    number = 0;
    if (prey_program == NULL || pred_program == NULL)
        return;
    g_list_foreach (creatures, sim_act, NULL);

    // purge dead creatures from simulation
    while (l != NULL)
    {
        GList *next = l->next;
        c = l->data;
        if (c->energy <= 0)
        {
            g_free (c);
            creatures = g_list_delete_link (creatures, l);
            number--;
        }
        l = next;
    }    
}

/*******************************************************************************
* Allows user interfaces to view the status of the program. This is done by 
* showing the distribution of creatures in the simulation. Therefore the 
* simulation will tell the status of each location for display.
*
* x         The x coord of the location.
* y         The y coord of the location.
* return    The status of the location.
*******************************************************************************/
LocationStatus 
sim_get_location_status (int x, int y)
{
    Location* location = &(locations[x][y]);
    if (location->prey_num > 0) 
        return PREY_HERE;
    if (location->pred_num > 0) 
        return PRED_HERE;
    if (location->prey_resting > 0) 
        return PREY_RESTING_HERE;
    if (location->pred_resting > 0) 
        return PRED_RESTING_HERE;
    return NOTHING_HERE;    
}

void
sim_check ()
{
    int i, j;
    Location *location;
    int location_number = 0;
    for (i = 0; i < SIZE; i++)
        for (j = 0; j < SIZE; j++)
        {
            location = &(locations[i][j]);
            location_number += location->prey_num;
            location_number += location->pred_num;
            location_number += location->prey_resting;
            location_number += location->pred_resting;
            
            if (location->prey_num < 0) 
                g_print ("insane prey_num\n");
            if (location->pred_num < 0) 
                g_print ("insane_pred_num\n");
            if (location->prey_breed < 0) 
            {
                g_print ("insane_prey_breed\n");
                g_print ("location %d,%d --\n", i, j);
            }
            if (location->pred_breed < 0) 
                g_print ("insane_pred_breed\n");
            if (location->prey_resting < 0) 
                g_print ("insane_prey_resting\n");
            if (location->pred_resting < 0) 
                g_print ("insane_pred_resting\n");
            if (location->prey_kill < 0) 
                g_print ("insane_prey_kill\n");

            if (location->prey_num < location->prey_breed)    
                g_print ("too many prey\n");
            if (location->prey_num < location->prey_breed)    
                g_print ("too many pred\n");
            if (location->prey_num < location->prey_kill)   
                g_print ("prey escaped\n");
            
        }

    if (location_number != number)
        g_print ("number mismatch\n");
}

void
simulation_free ()
{
    g_list_free_full (creatures, g_free);
    creatures = NULL;
    program_free (prey_program);
    program_free (pred_program);
}

/*******************************************************************************
* Initialises the simulation program. This will clear any program that already
* exists, create the new behavioural programs, reset the locations and create
* the new creatures.
*
* prey_file     The filename of the sim2 prey behaviour.
* pred_file     The filename of the sim2 predator behaviour.
* prey_num      The number of prey to create.
* pred_num      The number of predators to create.
*******************************************************************************/
void 
sim_reset (Program *prey, Program *pred, int prey_num, int pred_num)
{
    int i, j, x, y;
    Creature *creature;
    Location *location;

    GRand *random = g_rand_new ();
    simulation_free ();
    prey_program = prey;
    pred_program = pred;    

    for (i = 0; i < SIZE; i++)
        for (j = 0; j < SIZE; j++)
        {
            location = &(locations[i][j]);
            location->prey_num = 0;
            location->pred_num = 0;
            location->prey_breed = 0;
            location->pred_breed = 0;
            location->prey_resting = 0;
            location->pred_resting = 0;
            location->prey_kill = 0;
        }

    for (i = 0; i < prey_num; i++)
    {
        x = g_rand_int_range (random, 0, SIZE);  
        y = g_rand_int_range (random, 0, SIZE);
        creature = sim_creature_new (PREY, x, y);
        creatures = g_list_append (creatures, creature);
    }
    for (i = 0; i < pred_num; i++)
    {
        x = g_rand_int_range (random, 0, SIZE);  
        y = g_rand_int_range (random, 0, SIZE);
        creature = sim_creature_new (PRED, x, y);
        creatures = g_list_append (creatures, creature);
    }     
    g_rand_free (random);
}

