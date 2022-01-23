#include <stdlib.h>
#include <gtk/gtk.h>
#include "compiler.h"
#include "program.h"
#include "sim.h"

#define PX    5
#define BLUE  0,0,1
#define RED   1,0,0
#define WHITE 1,1,1
#define BLACK 0,0,0
#define APP_OPEN_FILE GTK_FILE_CHOOSER_ACTION_OPEN
#define ICO_SIZ GTK_ICON_SIZE_DIALOG

static gboolean run = FALSE;
static GtkWidget *prey_num_entry;
static GtkWidget *pred_num_entry;
static GtkWidget *prey_file_btn;
static GtkWidget *pred_file_btn;
static GtkWidget *info_bar;
static GtkWidget *message_label;
static GtkWidget *drawing_area;

//TODO: free at end of program, proper colours

/*******************************************************************************
* The callback function to execute an iteration of the simulation. If run is 
* TRUE it will run the a step of the simulation every 2 seconds. Else it will
* destroy the timed event.
*
* user_data     Is NULL.
* return        A boolean about whether to destroy the event or not.
*******************************************************************************/
static gboolean
app_iteration (gpointer user_data)
{
    if (run == TRUE)
    {
        sim_iteration ();
        sim_check ();
        gtk_widget_queue_draw (drawing_area);
    }
    return run;
}

/*******************************************************************************
* The callback function for the play button. It will create and run the timed
* events that execute the simulation.
*
* widget        The play button.
* user_data     Is NULL.
*******************************************************************************/
static void
app_play (GtkWidget *widget, gpointer user_data)
{
    if (run == FALSE)
    {
        run = TRUE;
        g_timeout_add (1000, app_iteration, NULL);
    }
}

/*******************************************************************************
* The callback function for the pause button. It will stop the execution of the
* simulation.
*
* widget        The pause button.
* user_data     Is NULL.
*******************************************************************************/
static void
app_pause (GtkWidget *widget, gpointer user_data)
{
    run = FALSE;
}

/*******************************************************************************
* Is the callback function for the refresh button. It attempts to create a new 
* simulation. If sucessful it will pause the timed events and recreate the 
* simulation.
*
* widget        The refresh button.
* user_data     Is NULL.
*******************************************************************************/
static void
app_config (GtkWidget *widget, gpointer user_data)
{
    int prey_num, pred_num;
    GFile *pred_file;
    GFile *prey_file;
    Program *prey;
    Program *pred;
    const char *error_message;
    const char *prey_num_str;
    const char *pred_num_str;
    
    pred_file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (pred_file_btn));
    prey_file = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (prey_file_btn));
    prey = compiler_get_program (prey_file);
    pred = compiler_get_program (pred_file);

    prey_num_str = gtk_entry_get_text (GTK_ENTRY (prey_num_entry));
    pred_num_str = gtk_entry_get_text (GTK_ENTRY (pred_num_entry));
    
    // TODO: check if pred and prey are actually numbers
    
    if (prey != NULL && pred != NULL)
    {
        run = FALSE;
        sim_reset (prey, pred, atoi (prey_num_str), atoi (pred_num_str));
        gtk_widget_queue_draw (drawing_area);
    }
    else
    {
        error_message = compiler_get_error_message ();
        gtk_label_set_text (GTK_LABEL (message_label), error_message);
        gtk_widget_show (info_bar);
    }
}

static gboolean
app_draw (GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
    int x, y;
    LocationStatus location_status;

    cairo_set_source_rgb (cr, BLACK);
    cairo_rectangle (cr, 0, 0, SIZE*PX + 2, SIZE*PX + 2);
    cairo_stroke (cr);

    for (x = 0; x < SIZE; x++)
        for (y = 0; y < SIZE; y++)
        {
            location_status = sim_get_location_status (x, y);
            switch (location_status)
            {
                case PREY_HERE: cairo_set_source_rgb (cr, BLUE); break;
                case PRED_HERE: cairo_set_source_rgb (cr, RED); break;
                case PREY_RESTING_HERE: cairo_set_source_rgb (cr, BLUE); break;
                case PRED_RESTING_HERE: cairo_set_source_rgb (cr, RED); break;
                default: cairo_set_source_rgb (cr, WHITE); break;
            }
            cairo_rectangle (cr, x*PX + 1, y*PX + 1, PX, PX);
            cairo_fill (cr);   
        }

    return FALSE;
}

static GtkWidget *
app_toolbar_new ()
{
    GtkWidget *toolbar;
    GtkWidget *play_btn;
    GtkWidget *pause_btn;
    GtkWidget *config_btn;
    GtkWidget *prey_label;
    GtkWidget *pred_label;
    GtkWidget *grid;

    toolbar = gtk_header_bar_new ();
    play_btn = gtk_button_new_from_icon_name ("media-playback-start", ICO_SIZ);
    pause_btn = gtk_button_new_from_icon_name ("media-playback-pause", ICO_SIZ);
    config_btn = gtk_button_new_from_icon_name ("view-refresh", ICO_SIZ);
    prey_file_btn = gtk_file_chooser_button_new ("Select File", APP_OPEN_FILE);
    pred_file_btn = gtk_file_chooser_button_new ("Select File", APP_OPEN_FILE);
    prey_num_entry = gtk_entry_new ();
    pred_num_entry = gtk_entry_new ();
    pred_label = gtk_label_new ("Predators: ");
    prey_label = gtk_label_new ("Prey: ");
    grid = gtk_grid_new ();

    gtk_grid_attach (GTK_GRID (grid), prey_label, 1, 1, 1, 1);
    gtk_grid_attach (GTK_GRID (grid), pred_label, 1, 2, 1, 1);
    gtk_grid_attach (GTK_GRID (grid), prey_num_entry, 2, 1, 1, 1);
    gtk_grid_attach (GTK_GRID (grid), pred_num_entry, 2, 2, 1, 1);
    gtk_grid_attach (GTK_GRID (grid), prey_file_btn, 3, 1, 1, 1);
    gtk_grid_attach (GTK_GRID (grid), pred_file_btn, 3, 2, 1, 1);
    gtk_header_bar_pack_start (GTK_HEADER_BAR (toolbar), play_btn);
    gtk_header_bar_pack_start (GTK_HEADER_BAR (toolbar), pause_btn);
    gtk_header_bar_pack_start (GTK_HEADER_BAR (toolbar), config_btn);
    gtk_header_bar_pack_start (GTK_HEADER_BAR (toolbar), grid);

    g_signal_connect (play_btn, "clicked", G_CALLBACK (app_play), NULL);
    g_signal_connect (pause_btn, "clicked", G_CALLBACK (app_pause), NULL);
    g_signal_connect (config_btn, "clicked", G_CALLBACK (app_config), NULL);

    //gtk_header_bar_set_show_close_button (GTK_HEADER_BAR (toolbar), TRUE);
    gtk_grid_set_row_spacing (GTK_GRID (grid), 5);
    gtk_grid_set_column_spacing (GTK_GRID (grid), 5);
    gtk_widget_set_halign (prey_label, GTK_ALIGN_END);

    return toolbar;
}

static void
app_activate (GApplication *app, gpointer user_data)
{
    GtkWidget *window;
    GtkWidget *toolbar;
    GtkWidget *scroll;
    GtkWidget *align;    
    GtkWidget *box;
    GtkWidget *content_area;
    
    /* create widgets */
    window = gtk_application_window_new (GTK_APPLICATION (app));
    toolbar = app_toolbar_new ();
    scroll = gtk_scrolled_window_new (NULL, NULL);
    //align = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
    drawing_area = gtk_drawing_area_new ();
    box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    info_bar = gtk_info_bar_new ();
    content_area = gtk_info_bar_get_content_area (GTK_INFO_BAR (info_bar));
    message_label = gtk_label_new ("This is a label");
    
    /* pack widgets */
    //gtk_container_add (GTK_CONTAINER (align), drawing_area);
    //gtk_container_add (GTK_CONTAINER (scroll), align);
    gtk_container_add (GTK_CONTAINER (scroll), drawing_area);
    gtk_box_pack_start (GTK_BOX (box), toolbar, FALSE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (box), info_bar, FALSE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (box), scroll, TRUE, TRUE, 0);
    gtk_container_add (GTK_CONTAINER (window), box);
    gtk_container_add (GTK_CONTAINER (content_area), message_label);
    
    /* connect signals */
    g_signal_connect (drawing_area, "draw", G_CALLBACK (app_draw), NULL);   
    g_signal_connect (info_bar, "response", G_CALLBACK (gtk_widget_hide), NULL);
    
    /* set properties of the widgets */
    gtk_window_set_default_size (GTK_WINDOW (window), 640, 640);
    gtk_window_set_title (GTK_WINDOW (window), "Predator-Prey Simulation");
    gtk_widget_set_size_request (drawing_area, SIZE*PX + 2, SIZE*PX + 2);
    gtk_info_bar_set_show_close_button (GTK_INFO_BAR (info_bar), TRUE);
    gtk_widget_show (message_label);
    gtk_widget_set_no_show_all (info_bar, TRUE);
    gtk_widget_show_all (window);
    gtk_widget_set_halign (drawing_area, GTK_ALIGN_CENTER);
    gtk_widget_set_valign (drawing_area, GTK_ALIGN_CENTER);
}

int 
main (int argc, char *argv[])
{
    GtkApplication *app;
    app = gtk_application_new ("org.steve.sim2app", G_APPLICATION_FLAGS_NONE);
    g_signal_connect (app, "activate", G_CALLBACK (app_activate), NULL);
    g_application_run (G_APPLICATION (app), argc, argv);
    simulation_free ();
    return 0;
}

