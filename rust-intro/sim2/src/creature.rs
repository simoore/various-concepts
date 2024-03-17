use std::rc::Rc;

use rand::Rng;

use crate::grid::{Grid, HasLocation};
use crate::program::{Action, Direction, Program};

/*****************************************************************************/
/********** CONSTANTS ********************************************************/
/*****************************************************************************/

/// The initial amount of energy for new creatues.
const INIT_ENERGY: i32 = 100;

/// The energy threshold requied to be available to breed.
const BREED_ENERGY_THRESHOLD: i32 = 25;

/*****************************************************************************/
/********** TYPES ************************************************************/
/*****************************************************************************/

#[derive(PartialEq, Copy, Clone)]
pub enum CreatureType {
    Predator,
    Prey,
}

pub struct Creature {
    creature_type: CreatureType,
    energy: i32,
    x_coord: i32,
    y_coord: i32,
    awake_daily: i32,
    count_down: i32,
    visible: bool,
    resting: bool,
    program: Rc<Program>,
}

/*****************************************************************************/
/********** FUNCTIONS: HasLocation Trait *************************************/
/*****************************************************************************/

impl HasLocation for Creature {
    fn xcoord(&self) -> i32 {
        self.x_coord
    }

    fn ycoord(&self) -> i32 {
        self.y_coord
    }
}

/*****************************************************************************/
/********** FUNCTIONS ********************************************************/
/*****************************************************************************/

impl Creature {
    pub fn new(
        creature_type: CreatureType,
        x: i32,
        y: i32,
        program: Rc<Program>,
        grid: &mut Grid,
    ) -> Creature {
        let creature = Creature {
            creature_type,
            energy: INIT_ENERGY,
            x_coord: x,
            y_coord: y,
            awake_daily: 0,
            count_down: 0,
            visible: true,
            resting: false,
            program,
        };

        match creature.creature_type {
            CreatureType::Predator => {
                grid.location(&creature).add_predator(creature.energy);
            }
            CreatureType::Prey => {
                grid.location(&creature).add_prey(creature.energy);
            }
        }
        creature
    }

    /// The creature is dead is energy reaches 0.
    pub fn is_dead(&self) -> bool {
        self.energy <= 0
    }

    /// When a creature changes energy, we have to check if it has enough energy to breed and flag that with the
    /// location it is in.
    ///
    /// @param delta_energy
    ///     The change in energy for the creature.
    /// @param grid
    ///     The simulation grid.
    fn change_energy(&mut self, delta_energy: i32, grid: &mut Grid) {
        let original_energy = self.energy;
        self.energy += delta_energy;
        if original_energy > BREED_ENERGY_THRESHOLD && self.energy <= BREED_ENERGY_THRESHOLD {
            match self.creature_type {
                CreatureType::Predator => grid.location(self).remove_breeding_predator(),
                CreatureType::Prey => grid.location(self).remove_breeding_prey(),
            }
        } else if original_energy <= BREED_ENERGY_THRESHOLD && self.energy > BREED_ENERGY_THRESHOLD
        {
            match self.creature_type {
                CreatureType::Predator => grid.location(self).add_breeding_predator(),
                CreatureType::Prey => grid.location(self).add_breeding_prey(),
            }
        }
    }

    /// Setting the creature to invisible change its visibility in the location it is in now.
    fn set_invisible(&mut self, grid: &mut Grid) {
        match self.creature_type {
            CreatureType::Predator => grid.location(self).rest_predator(self.energy),
            CreatureType::Prey => grid.location(self).rest_prey(self.energy),
        }
        self.visible = false;
    }

    /// Setting the creature to visible change its visibility in the location it is in now.
    fn set_visible(&mut self, grid: &mut Grid) {
        match self.creature_type {
            CreatureType::Predator => grid.location(self).awake_predator(self.energy),
            CreatureType::Prey => grid.location(self).awake_prey(self.energy),
        }
        self.visible = true;
    }

    /// Moves a creature from one location to another.
    fn movee(&mut self, dir: Direction, grid: &mut Grid) {
        // Remove creature from location.
        match self.creature_type {
            CreatureType::Predator => grid.location(self).remove_predator(self.energy),
            CreatureType::Prey => grid.location(self).remove_prey(self.energy),
        }

        // It consumes on energy to move.
        self.energy -= 1;

        // Adjust coordinates based on direction of move.
        match dir {
            Direction::NW | Direction::N | Direction::NE => self.y_coord -= 1,
            Direction::SW | Direction::S | Direction::SE => self.y_coord += 1,
            _ => (),
        }
        match dir {
            Direction::NE | Direction::E | Direction::SE => self.x_coord += 1,
            Direction::NW | Direction::W | Direction::SW => self.x_coord -= 1,
            _ => (),
        }

        // Wrap coordinates if they are out of bounds.
        if self.x_coord >= (grid.xsize() as i32) {
            self.x_coord = 0;
        } else if self.x_coord < 0 {
            self.x_coord = (grid.xsize() as i32) - 1;
        }
        if self.y_coord >= (grid.ysize() as i32) {
            self.y_coord = 0;
        } else if self.y_coord < 0 {
            self.y_coord = (grid.ysize() as i32) - 1;
        }

        // Add creature from location.
        match self.creature_type {
            CreatureType::Predator => grid.location(self).add_predator(self.energy),
            CreatureType::Prey => grid.location(self).add_prey(self.energy),
        }
    }

    /// Resting makes the creature invisible in the location.
    fn rest(&mut self, count: i32, grid: &mut Grid) {
        self.count_down = count;
        self.set_invisible(grid);
        if self.count_down < 8 {
            self.energy -= 1;
        } else if self.count_down >= 24 {
            self.energy -= 5;
        }
        self.awake_daily = 0;
    }

    /// When prey hunts, it is always successful since it just grazes. Predators on the other hand have a random change
    /// to hunt prey and only if prey exists in the location.
    fn hunt(&mut self, dir: Direction, grid: &mut Grid) {
        self.movee(dir, grid);
        if self.creature_type == CreatureType::Prey {
            self.change_energy(2, grid);
        } else if grid.location(self).has_prey() {
            let mut rng = rand::thread_rng();
            if self.energy > rng.gen_range(0..100) {
                self.change_energy(25, grid);
                self.count_down = 3;
                grid.location(self).kill_prey();
            }
            self.change_energy(-10, grid);
        }
    }

    fn breed(&mut self, dir: Direction, grid: &mut Grid) -> Vec<Creature> {
        self.movee(dir, grid);

        let has_partner = match self.creature_type {
            CreatureType::Predator => grid.location(self).predator_has_partner(),
            CreatureType::Prey => grid.location(self).prey_has_partner(),
        };

        if has_partner {
            self.count_down = 6;
            self.set_invisible(grid);
            self.energy -= 25;

            let mut baby1 = Creature::new(
                self.creature_type,
                self.x_coord,
                self.y_coord,
                self.program.clone(),
                grid,
            );
            let mut baby2 = Creature::new(
                self.creature_type,
                self.x_coord,
                self.y_coord,
                self.program.clone(),
                grid,
            );

            baby1.rest(6, grid);
            baby2.rest(6, grid);
            vec![baby1, baby2]
        } else {
            Vec::new()
        }
    }

    fn is_eaten(&mut self, grid: &mut Grid) -> bool {
        if self.creature_type == CreatureType::Prey
            && grid.location(self).has_killed_prey()
            && self.visible
        {
            self.change_energy(-self.energy, grid);
            grid.location(self).remove_killed_prey();
            return true;
        }
        false
    }

    pub fn act(&mut self, grid: &mut Grid) -> Vec<Creature> {
        // We count down to 0 to perform the next action.
        if self.count_down > 0 {
            self.count_down -= 1;
        }

        // If not resting increment awake_daily.
        if !self.resting {
            self.awake_daily += 1;
        }

        // Check if a predator has eaten prey at this location - you may the unlucky creature to be killed.
        if self.is_eaten(grid) {
            return Vec::new();
        }

        if self.count_down == 0 {
            // Long actions macke the creature invisible. When we are about to do a new action the creature becomes
            // visible again.
            if !self.visible {
                self.set_visible(grid);
            }

            let new_action = self.program.execute(self.energy, self.awake_daily);

            let new_creatures: Vec<Creature> = match new_action {
                Action::Move(dir) => {
                    self.movee(dir, grid);
                    Vec::new()
                }
                Action::Rest(count) => {
                    self.rest(count, grid);
                    Vec::new()
                }
                Action::Hunt(dir) => {
                    self.hunt(dir, grid);
                    Vec::new()
                }
                Action::Breed(dir) => self.breed(dir, grid),
                _ => Vec::new(),
            };

            // If the creature has run out of energy it has died and it should be removed from the grid. It will be
            // purged from the simulation later.
            if self.is_dead() {
                if self.visible {
                    match self.creature_type {
                        CreatureType::Predator => grid.location(self).remove_predator(0),
                        CreatureType::Prey => grid.location(self).remove_prey(0),
                    }
                } else {
                    match self.creature_type {
                        CreatureType::Predator => grid.location(self).remove_resting_predator(),
                        CreatureType::Prey => grid.location(self).remove_resting_prey(),
                    }
                }
            }
            return new_creatures;
        }
        Vec::new()
    }
}
