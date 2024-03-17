use std::fs::read_to_string;
use std::path::{Path, PathBuf};
use std::rc::Rc;

use rand::Rng;

use crate::compiler::get_program;
use crate::creature::{Creature, CreatureType};
use crate::grid::Grid;

/*****************************************************************************/
/********** SIMULATION TYPE **************************************************/
/*****************************************************************************/

pub struct Simulation {
    prey_filename: Option<PathBuf>,
    pred_filename: Option<PathBuf>,
    run: bool,
    creatures: Vec<Creature>,
    valid_configuration: bool,
    grid: Grid,
}

impl Simulation {
    pub fn new(xsize: usize, ysize: usize) -> Simulation {
        Simulation {
            prey_filename: None,
            pred_filename: None,
            run: false,
            valid_configuration: false,
            creatures: Vec::new(),
            grid: Grid::new(xsize, ysize),
        }
    }

    pub fn grid(&self) -> &Grid {
        &self.grid
    }

    pub fn set_prey_filename(&mut self, path: &Path) {
        self.prey_filename = Some(path.to_path_buf());
    }

    pub fn set_predator_filename(&mut self, path: &Path) {
        self.pred_filename = Some(path.to_path_buf());
    }

    pub fn set_run(&mut self, val: bool) {
        self.run = val && self.valid_configuration;
    }

    pub fn iteration(&mut self) -> bool {
        if self.run {
            let mut new_creatures = self
                .creatures
                .iter_mut()
                .flat_map(|c| c.act(&mut self.grid))
                .collect::<Vec<Creature>>();
            self.creatures.retain(|c| !c.is_dead());
            self.creatures.append(&mut new_creatures);
        }
        self.run
    }

    pub fn config(&mut self, n_pred: i32, n_prey: i32) -> Option<String> {
        let prey_path = match &self.prey_filename {
            Some(s) => s,
            None => {
                return Some("Invalid Prey Program Path".to_string());
            }
        };

        let pred_path = match &self.pred_filename {
            Some(s) => s,
            None => {
                return Some("Invalid Predator Program Path".to_string());
            }
        };

        let prey_contents = match read_to_string(prey_path) {
            Ok(s) => s,
            Err(_) => {
                return Some("Prey Program IO Error".to_string());
            }
        };

        let prey_program = match get_program(&prey_contents) {
            Ok(p) => Rc::new(p),
            Err(e) => {
                return Some(format!("Prey compiler error {:?}", e));
            }
        };

        let pred_contents = match read_to_string(pred_path) {
            Ok(s) => s,
            Err(_) => {
                return Some("Predator Program IO Error".to_string());
            }
        };

        let pred_program = match get_program(&pred_contents) {
            Ok(p) => Rc::new(p),
            Err(e) => {
                return Some(format!("Predator compiler error {:?}", e));
            }
        };

        // Clear all existing creatures and create new ones.
        self.creatures.clear();

        let mut rng = rand::thread_rng();
        for _ in 0..n_pred {
            let x = rng.gen_range(0..100);
            let y = rng.gen_range(0..100);
            let c = Creature::new(
                CreatureType::Predator,
                x,
                y,
                pred_program.clone(),
                &mut self.grid,
            );
            self.creatures.push(c);
        }
        for _ in 0..n_prey {
            let x = rng.gen_range(0..100);
            let y = rng.gen_range(0..100);
            let c = Creature::new(
                CreatureType::Prey,
                x,
                y,
                prey_program.clone(),
                &mut self.grid,
            );
            self.creatures.push(c);
        }

        // We now have a valid simulation configured.
        self.valid_configuration = true;
        None
    }
}
