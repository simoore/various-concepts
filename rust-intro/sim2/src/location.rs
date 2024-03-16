#[derive(Clone)]
pub struct Location {
    prey_num: i32,
    pred_num: i32,
    prey_breed: i32,
    pred_breed: i32,
    prey_resting: i32,
    pred_resting: i32,
    prey_kill: i32,
}

#[derive(Debug)]
pub enum LocationStatus {
    PreyHere,
    PredHere, 
    PreyRestingHere, 
    PredRestingHere,
    NothingHere,
}

impl Location {
    pub fn new() -> Location {
        Location {
            prey_num: 0,
            pred_num: 0,
            prey_breed: 0,
            pred_breed: 0,
            prey_resting: 0,
            pred_resting: 0,
            prey_kill: 0,
        }
    }

    pub fn has_prey(&self) -> bool{
        self.prey_num > 0
    }

    pub fn has_killed_prey(&self) -> bool {
        self.prey_kill > 0
    }

    pub fn prey_has_partner(&self) -> bool {
        self.prey_breed > 1
    }

    pub fn predator_has_partner(&self) -> bool {
        self.pred_breed > 1
    }

    pub fn add_breeding_predator(&mut self) {
        self.pred_breed += 1;
    }

    pub fn add_breeding_prey(&mut self) {
        self.prey_breed += 1;
    }

    pub fn remove_breeding_predator(&mut self) {
        self.pred_breed -= 1;
    }

    pub fn remove_breeding_prey(&mut self) {
        self.prey_breed -= 1;
    }

    pub fn add_predator(&mut self, energy: i32) {
        self.pred_num += 1;
        if energy > 25 {
            self.pred_breed +=1;
        }
    }

    pub fn add_prey(&mut self, energy: i32) {
        self.prey_num += 1;
        if energy > 25 {
            self.prey_breed +=1;
        }
    }

    pub fn remove_predator(&mut self, energy: i32) {
        self.pred_num -= 1;
        if energy > 25 {
            self.pred_breed -=1;
        }
    }

    pub fn remove_prey(&mut self, energy: i32) {
        self.prey_num -= 1;
        if energy > 25 {
            self.prey_breed -=1;
        }
    }

    pub fn add_resting_predator(&mut self) {
        self.pred_resting += 1;
    }

    pub fn add_resting_prey(&mut self) {
        self.prey_resting += 1;
    }

    pub fn remove_resting_predator(&mut self) {
        self.pred_resting -= 1;
    }

    pub fn remove_resting_prey(&mut self) {
        self.prey_resting -= 1;
    }

    pub fn kill_prey(&mut self) {
        self.prey_kill += 1
    }

    pub fn remove_killed_prey(&mut self) {
        self.prey_num -= 1;
        self.prey_kill -= 1;
    }

    pub fn rest_predator(&mut self, energy: i32) {
        self.remove_predator(energy);
        self.add_resting_predator();
    }

    pub fn rest_prey(&mut self, energy: i32) {
        self.remove_prey(energy);
        self.add_resting_prey();
    }

    pub fn awake_predator(&mut self, energy: i32) {
        self.add_predator(energy);
        self.remove_resting_predator();
    }


    /// When a prey awakens it is removed from the resting count, and added back into the prey count.
    /// 
    /// @param energy
    ///     If the creatures energy is above a threshold, it is also added to the breed count.
    pub fn awake_prey(&mut self, energy: i32) {
        self.add_prey(energy);
        self.remove_resting_prey();
    }


    /// The location status is used to visualize the location on the UI.
    ///
    /// @return
    ///     The location status, that is, if the location contains predators or prey.
    pub fn status(&self) -> LocationStatus {
        if self.pred_num > 0 { return LocationStatus::PredHere; }
        if self.prey_num > 0 { return LocationStatus::PreyHere; }
        if self.pred_resting > 0 { return LocationStatus::PredRestingHere; }
        if self.prey_resting > 0 { return LocationStatus::PreyRestingHere; }
        LocationStatus::NothingHere    
    }
    
}