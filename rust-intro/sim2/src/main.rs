use std::cell::RefCell;
use std::rc::Rc;
use std::time::Duration;

use gtk4 as gtk;
use gtk::prelude::*;
use gtk::cairo;
use gtk::glib;
use gtk::gio;

use crate::simulation::Simulation;

// For top-level rust files, you need to explicity include the source files that are part of this module.
mod code;
mod compiler;
mod creature;
mod grid;
mod lexer;
mod location;
mod program;
mod simulation;

// Attempts to start the simulation. It tells the simulation and executes one iteration. If the simulation isn't
// ready, the timer will be told to break after. One reason the simulation may not be ready is that it hasn't 
// received valid sim2 files for the predator and prey behaviour.
//
// @param sim
//      A reference to the simulation we are attempting to start.
fn play(sim: Rc<RefCell<Simulation>>) {
    sim.borrow_mut().set_run(true);
    let timer_sim = sim.clone();
    glib::timeout_add_local(Duration::from_millis(1000), move || {
        if timer_sim.borrow_mut().iteration() { glib::ControlFlow::Continue } else { glib::ControlFlow::Break }
    });
}

// Pauses the simulation be setting the simulation run flag to false.
//
// @param sim
//      The reference to the simulation that we are pausing.
fn pause(sim: Rc<RefCell<Simulation>>) {
    sim.borrow_mut().set_run(false);
}

// Redraws the drawing area after each iteration.
fn draw(area: &gtk::DrawingArea, cairo: &cairo::Context, width: i32, height: i32) {
    println!("draw");
}

// This is called when an open file button is pressed for either the predator or prey sim2 file. 
// 
// @param sim
//      The simulation reference that is going to process the sim2 files.
// @param prey
//      If true, we are opening the sim2 file for prey, otherwise it is for predators.
fn launch_file_dialog(sim: Rc<RefCell<Simulation>>, prey: bool) {

    let filedialog = gtk::FileDialog::builder()
        .title(if prey { "Open prey sim2 file" } else {"Open predator sim2 file"})
        .accept_label("Ok")
        .build();

    let a: Option<&gtk::ApplicationWindow> = None;
    filedialog.open(a, gio::Cancellable::NONE, move |file| {
        if let Ok(file) = file {
            let filename = file.path().expect("Couldn't get file path");
            if prey {
                sim.borrow_mut().set_prey_filename(&filename);
            } else {
                sim.borrow_mut().set_predator_filename(&filename);
            }
            println!("Opened file {}", filename.display());
        }
    });
}

// This initializes the UI toolbar which contains the user controls.
//
// @param sim
//      The mutable reference to the simulation this UI manages.
// @return
//      The widget container that makes up the toolbar.
fn build_toolbar(sim: Rc<RefCell<Simulation>>) -> gtk::Box {
    let play_button = gtk::Button::builder().icon_name("media-playback-start").build();
    let pause_button = gtk::Button::builder().icon_name("media-playback-pause").build();
    let config_button = gtk::Button::builder().icon_name("view-refresh").build();
    
    let play_sim = sim.clone();
    play_button.connect_clicked(move |_| { play(play_sim.clone()) });
    let pause_sim = sim.clone();
    pause_button.connect_clicked(move |_| { pause(pause_sim.clone())  });
    let config_sim = sim.clone();
    config_button.connect_clicked(move |_| { config_sim.borrow_mut().config(5, 5) });

    let grid = gtk::Grid::builder().row_spacing(5).column_spacing(5).build();
    let prey_label = gtk::Label::builder().label("Prey:").halign(gtk::Align::End).build();
    let pred_label = gtk::Label::builder().label("Predators:").build();
    let prey_file_chooser = gtk::Button::builder().label("Select File").build();
    let pred_file_chooser = gtk::Button::builder().label("Select File").build();
    let prey_entry = gtk::Entry::builder().build();
    let pred_entry = gtk::Entry::builder().build();

    let prey_sim_clone = sim.clone();
    prey_file_chooser.connect_clicked(move |_| { launch_file_dialog(prey_sim_clone.clone(), true) });
    let pred_sim_clone = sim.clone();
    pred_file_chooser.connect_clicked(move |_| { launch_file_dialog(pred_sim_clone.clone(), false) });

    grid.attach(&prey_label, 0, 0, 1, 1);
    grid.attach(&pred_label, 0, 1, 1, 1);
    grid.attach(&prey_file_chooser, 1, 0, 1, 1);
    grid.attach(&pred_file_chooser, 1, 1, 1, 1);
    grid.attach(&prey_entry, 2, 0, 1, 1);
    grid.attach(&pred_entry, 2, 1, 1, 1);

    let toolbar = gtk::Box::builder()
        .margin_bottom(5)
        .margin_end(5)
        .margin_start(5)
        .margin_top(5)
        .orientation(gtk::Orientation::Horizontal)
        .spacing(5)
        .build();
    toolbar.append(&play_button);
    toolbar.append(&pause_button);
    toolbar.append(&config_button);
    toolbar.append(&grid);
    toolbar
}

// This initializes the application window.
//
// @param sim
//      The predator/prey simulator that is modified by various events in the gtk application.
// @param app
//      Used to link the window to the application.
fn build_ui(sim: Rc<RefCell<Simulation>>, app: &gtk::Application) {
    
    // Create the main components in the application window.
    let toolbar = build_toolbar(sim);
    let drawing_area = gtk::DrawingArea::builder().build();
    let scrolled_window = gtk::ScrolledWindow::builder().child(&drawing_area).build();
    let sep = gtk::Separator::builder().build();
    let boxx = gtk::Box::builder().spacing(5).orientation(gtk::Orientation::Vertical).build();

    // We create the main window.
    let window = gtk::ApplicationWindow::builder()
        .application(app)
        .default_width(640)
        .default_height(640)
        .child(&boxx)
        .title("Predator-Prey Simulation")
        .build();

    // Set the drawing function that is called each time the GUI wants to redraw the drawing area.
    drawing_area.set_draw_func(draw);

    // Add widgets to the gtk box.
    boxx.append(&toolbar);
    boxx.append(&sep);
    boxx.append(&scrolled_window);

    // Show the window.
    window.present();
}

// This is the main application of the predator/prey simulation.
//
// @return
//      The glib application exit code.
fn main() -> glib::ExitCode {

    // This is the stack allocated simulation memory. We move it around the application using a reference counter.
    // RefCell allows us to override compile time ownership of mutable data and instead we perform run-time checks
    // of ownership. This allows all the event driven logic in this application to own a reference to the mutable
    // simulation.
    let mut sim = Rc::new(RefCell::new(Simulation::new()));

    // To move an Rc or Arc into a closure we have to clone it first
    let mut sim_clone = sim.clone();

    // Create the application and link to the function that initializes the UI.
    let app = gtk::Application::builder().application_id("org.steve.sim2app").build();
    app.connect_activate(move |app| build_ui(sim_clone.clone(), app));
    app.run()
}