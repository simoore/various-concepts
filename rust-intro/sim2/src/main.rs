use std::cell::RefCell;
use std::rc::Rc;
use std::time::Duration;

use gtk::cairo;
use gtk::gio;
use gtk::glib;
use gtk::prelude::*;
use gtk4 as gtk;

use crate::location::LocationStatus;
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

/*****************************************************************************/
/********** CONSTANTS ********************************************************/
/*****************************************************************************/

const PX: f64 = 5.0;
const XGRID: i32 = 100;
const YGRID: i32 = 100;

/*****************************************************************************/
/********** SIMULATION UI ****************************************************/
/*****************************************************************************/

/// Stores UI widgets so a reference to them can be passed around to the UI event callbacks.
struct SimUi {
    drawing_area: gtk::DrawingArea,
    scrolled_window: gtk::ScrolledWindow,
    sep: gtk::Separator,
    boxx: gtk::Box,
    window: gtk::ApplicationWindow,
    prey_entry: gtk::Entry,
    pred_entry: gtk::Entry,
}

impl SimUi {
    /// Constructs a new SimUI. We group these together so a refernce to them can be based around to event driven
    /// objects in the application.
    ///
    /// @param app
    ///     The Gtk application object.
    /// @return
    ///     The structure of UI widgets.
    fn new(app: &gtk::Application) -> SimUi {
        let boxx = gtk::Box::builder()
            .spacing(5)
            .orientation(gtk::Orientation::Vertical)
            .build();
        let sep = gtk::Separator::builder().build();
        let drawing_area = gtk::DrawingArea::builder()
            .content_height(502)
            .content_width(502)
            .hexpand(true)
            .vexpand(true)
            .halign(gtk::Align::Center)
            .valign(gtk::Align::Center)
            .build();
        let scrolled_window = gtk::ScrolledWindow::builder()
            .child(&drawing_area)
            .vscrollbar_policy(gtk::PolicyType::Always)
            .hscrollbar_policy(gtk::PolicyType::Always)
            .has_frame(true)
            .build();
        let prey_entry = gtk::Entry::builder().build();
        let pred_entry = gtk::Entry::builder().build();

        // We create the main window.
        let window = gtk::ApplicationWindow::builder()
            .application(app)
            .default_width(640)
            .default_height(640)
            .child(&boxx)
            .title("Predator-Prey Simulation")
            .build();

        SimUi {
            drawing_area,
            scrolled_window,
            sep,
            boxx,
            window,
            prey_entry,
            pred_entry,
        }
    }
}

/*****************************************************************************/
/********** APPLICATION FUNCTIONS ********************************************/
/*****************************************************************************/

/// Attempts to start the simulation. It tells the simulation and executes one iteration. If the simulation isn't
/// ready, the timer will be told to break after. One reason the simulation may not be ready is that it hasn't
/// received valid sim2 files for the predator and prey behaviour.
///
/// @param sim
///     A reference to the simulation we are attempting to start.
fn play(simui: Rc<RefCell<SimUi>>, sim: Rc<RefCell<Simulation>>) {
    sim.borrow_mut().set_run(true);
    let timer_sim = sim.clone();
    glib::timeout_add_local(Duration::from_millis(1000), move || {
        let cont = timer_sim.borrow_mut().iteration();
        simui.borrow().drawing_area.queue_draw();
        if cont {
            glib::ControlFlow::Continue
        } else {
            glib::ControlFlow::Break
        }
    });
}

/// Pauses the simulation be setting the simulation run flag to false.
///
/// @param sim
///     The reference to the simulation that we are pausing.
fn pause(sim: Rc<RefCell<Simulation>>) {
    sim.borrow_mut().set_run(false);
}

/// Prints a message in a dialog box. Typically used to display errors when configuring a simulation.
///
/// @param simui
///     Reference to the simulation UI objects.
/// @param msg
///     The message to print.
fn show_message(simui: &SimUi, msg: &str) {
    gtk::AlertDialog::builder()
        .message(msg)
        .build()
        .show(Some(&simui.window));
}

/// Configures the simulation. This parses the pred and prey entries to make sure they are positive integers and then
/// asks the simulation to setup. If that is successful, the simulation is ready to run.
///
/// @param simui
///     Reference to the UI widgets.
/// @param sim
///     Reference to the simulation.
fn config(simui: Rc<RefCell<SimUi>>, sim: Rc<RefCell<Simulation>>) {
    let pred_num = match simui.borrow().pred_entry.text().parse::<i32>() {
        Ok(n) => n,
        Err(_) => {
            show_message(&simui.borrow(), "Error parsing pred num");
            return;
        }
    };

    let prey_num = match simui.borrow().prey_entry.text().parse::<i32>() {
        Ok(n) => n,
        Err(_) => {
            show_message(&simui.borrow(), "Error parseing prey num");
            return;
        }
    };

    if let Some(s) = sim.borrow_mut().config(pred_num, prey_num) {
        show_message(&simui.borrow(), &s);
    };

    simui.borrow().drawing_area.queue_draw();
}

/// Redraws the drawing area after each iteration.
///
/// @param sim
///     The simulation handle to know how to draw the state of the simulation.
/// @param cairo
///     The handle to the cairo drawing backend which draws the widget.
fn draw(
    sim: Rc<RefCell<Simulation>>,
    _area: &gtk::DrawingArea,
    cairo: &cairo::Context,
    _width: i32,
    _height: i32,
) {
    let xsize = f64::from(XGRID);
    let ysize = f64::from(YGRID);

    cairo.set_source_rgb(0.0, 0.0, 0.0);
    cairo.rectangle(0.0, 0.0, PX * xsize + 2.0, ysize * PX + 2.0);
    let _ = cairo.stroke();

    for x in 0..100 {
        for y in 0..100 {
            match sim.borrow().grid().loc(x, y).status() {
                LocationStatus::PreyHere => cairo.set_source_rgb(0.0, 0.0, 1.0),
                LocationStatus::PredHere => cairo.set_source_rgb(1.0, 0.0, 0.0),
                LocationStatus::PreyRestingHere => cairo.set_source_rgb(0.5, 0.5, 1.0),
                LocationStatus::PredRestingHere => cairo.set_source_rgb(1.0, 0.5, 0.5),
                LocationStatus::Nothing => cairo.set_source_rgb(1.0, 1.0, 1.0),
            };
            cairo.rectangle(f64::from(x) * PX + 1.0, f64::from(y) * PX + 1.0, PX, PX);
            let _ = cairo.fill();
        }
    }
}

/// This is called when an open file button is pressed for either the predator or prey sim2 file.
///
/// @param sim
///     The simulation reference that is going to process the sim2 files.
/// @param prey
///     If true, we are opening the sim2 file for prey, otherwise it is for predators.
fn launch_file_dialog(sim: Rc<RefCell<Simulation>>, prey: bool) {
    let filedialog = gtk::FileDialog::builder()
        .title(if prey {
            "Open prey sim2 file"
        } else {
            "Open predator sim2 file"
        })
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

/// This initializes the UI toolbar which contains the user controls.
///
/// @param simui
///     Reference to the UI widgets.
/// @param sim
///     The mutable reference to the simulation this UI manages.
/// @return
///     The widget container that makes up the toolbar.
fn build_toolbar(simui: Rc<RefCell<SimUi>>, sim: Rc<RefCell<Simulation>>) -> gtk::Box {
    let play_button = gtk::Button::builder()
        .icon_name("media-playback-start")
        .build();
    let pause_button = gtk::Button::builder()
        .icon_name("media-playback-pause")
        .build();
    let config_button = gtk::Button::builder().icon_name("view-refresh").build();

    let grid = gtk::Grid::builder()
        .row_spacing(5)
        .column_spacing(5)
        .build();
    let prey_label = gtk::Label::builder()
        .label("Prey:")
        .halign(gtk::Align::End)
        .build();
    let pred_label = gtk::Label::builder().label("Predators:").build();
    let prey_file_chooser = gtk::Button::builder().label("Select File").build();
    let pred_file_chooser = gtk::Button::builder().label("Select File").build();

    grid.attach(&prey_label, 0, 0, 1, 1);
    grid.attach(&pred_label, 0, 1, 1, 1);
    grid.attach(&prey_file_chooser, 1, 0, 1, 1);
    grid.attach(&pred_file_chooser, 1, 1, 1, 1);
    grid.attach(&simui.borrow().prey_entry, 2, 0, 1, 1);
    grid.attach(&simui.borrow().pred_entry, 2, 1, 1, 1);

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

    // To move an Rc or Arc into a closure we have to clone it if we want to use it later in a function.
    let play_sim = sim.clone();
    let play_simui = simui.clone();
    play_button.connect_clicked(move |_| play(play_simui.clone(), play_sim.clone()));
    let pause_sim = sim.clone();
    pause_button.connect_clicked(move |_| pause(pause_sim.clone()));
    let config_sim = sim.clone();
    config_button.connect_clicked(move |_| config(simui.clone(), config_sim.clone()));

    let prey_sim_clone = sim.clone();
    prey_file_chooser.connect_clicked(move |_| launch_file_dialog(prey_sim_clone.clone(), true));
    let pred_sim_clone = sim.clone();
    pred_file_chooser.connect_clicked(move |_| launch_file_dialog(pred_sim_clone.clone(), false));

    toolbar
}

/// This initializes the application window.
///
/// @param sim
///     The predator/prey simulator that is modified by various events in the gtk application.
/// @param app
///     Used to link the window to the application.
fn build_ui(sim: Rc<RefCell<Simulation>>, app: &gtk::Application) {
    // Create the main components in the application window.
    let simui = Rc::new(RefCell::new(SimUi::new(app)));
    let toolbar = build_toolbar(simui.clone(), sim.clone());

    // Set the drawing function that is called each time the GUI wants to redraw the drawing area.
    simui
        .borrow()
        .drawing_area
        .set_draw_func(move |a, c, w, h| draw(sim.clone(), a, c, w, h));

    // Add widgets to the gtk box.
    simui.borrow().boxx.append(&toolbar);
    simui.borrow().boxx.append(&simui.borrow().sep);
    simui.borrow().boxx.append(&simui.borrow().scrolled_window);

    // Show the window.
    simui.borrow().window.present();
}

/// This is the main application of the predator/prey simulation.
///
/// @return
///     The glib application exit code.
fn main() -> glib::ExitCode {
    // This is the stack allocated simulation memory. We move it around the application using a reference counter.
    // RefCell allows us to override compile time ownership of mutable data and instead we perform run-time checks
    // of ownership. This allows all the event driven logic in this application to own a reference to the mutable
    // simulation.
    let sim = Rc::new(RefCell::new(Simulation::new(
        usize::try_from(XGRID).unwrap(),
        usize::try_from(YGRID).unwrap(),
    )));

    // Create the application and link to the function that initializes the UI.
    let app = gtk::Application::builder()
        .application_id("org.steve.sim2app")
        .build();
    app.connect_activate(move |app| build_ui(sim.clone(), app));
    app.run()
}
