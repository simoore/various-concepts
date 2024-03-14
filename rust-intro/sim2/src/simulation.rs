use std::path::PathBuf;

use crate::creature::Creature;
use crate::grid::Grid;

pub struct Simulation {
    prey_filename: Option<PathBuf>,
    pred_filename: Option<PathBuf>,
    run: bool,
    n_prey: i32,
    n_pred: i32,
    creatures: Vec<Creature>,
    valid_configuration: bool,
    grid: Grid
}

impl Simulation {
    
    pub fn new() -> Simulation {
        Simulation {
            n_prey: -1,
            n_pred: -1,
            prey_filename: None,
            pred_filename: None,
            run: false,
            valid_configuration: false,
            creatures: Vec::new(),
            grid: Grid::new(100, 100),
        }
    }

    pub fn set_prey_filename(&mut self, path: &PathBuf) {
        self.prey_filename = Some(path.clone());
    }

    pub fn set_predator_filename(&mut self, path: &PathBuf) {
        self.pred_filename = Some(path.clone());
    }

    pub fn set_run(&mut self, val: bool) {
        self.run = val && self.valid_configuration;
    }

    pub fn iteration(&mut self) -> bool {
        if self.run {
            let mut new_creatures = self.creatures
                .iter_mut()
                .map(|c| c.act(&mut self.grid))
                .flatten()
                .collect::<Vec<Creature>>();
            self.creatures.retain(|c| !c.is_dead());
            self.creatures.append(&mut new_creatures);
        }
        self.run
    }

    pub fn check(&self) {

    }

    pub fn config(&mut self, n_pred: i32, n_prey: i32) {
        // let prey_file = File::open(self.prey_filename).expect("Couldn't open file");
        // let pred_file = File::open(self.pred_filename).expect("Couldn't open file");

        // let mut reader = BufReader::new(file);
        // let mut contents = String::new();
        // let _ = reader.read_to_string(&mut contents);

        // let pre

        // text_view.buffer().set_text(&contents);

        // if 
    }
}
