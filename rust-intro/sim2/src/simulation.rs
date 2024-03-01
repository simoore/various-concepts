use std::path::PathBuf;

pub struct Simulation {
    prey_filename: Option<PathBuf>,
    pred_filename: Option<PathBuf>,
    run: bool,
    n_prey: i32,
    n_pred: i32,
}

impl Simulation {
    
    pub fn new() -> Simulation {
        Simulation {
            n_prey: -1,
            n_pred: -1,
            prey_filename: None,
            pred_filename: None,
            run: false,
        }
    }

    pub fn set_prey_filename(&mut self, path: &PathBuf) {
        self.prey_filename = Some(path.clone());
    }

    pub fn set_predator_filename(&mut self, path: &PathBuf) {
        self.pred_filename = Some(path.clone());
    }

    pub fn set_run(&mut self, val: bool) {
        self.run = val;
    }

    pub fn iteration(&self) -> bool {
        match (&self.prey_filename, &self.pred_filename) {
            (Some(x), Some(y)) => println!("play both"),
            (Some(x), None) => println!("play x"),
            (None, Some(y)) => println!("play y"),
            _ => println!("play"),
        }
        println!("Sim play");
        self.run
    }

    pub fn config(&mut self) {
        //let file = File::open(filename).expect("Couldn't open file");

        // let mut reader = BufReader::new(file);
        // let mut contents = String::new();
        // let _ = reader.read_to_string(&mut contents);

        // text_view.buffer().set_text(&contents);
    }

    // // This function performs one iteration of the simulation. This involves asking each creature their intention which 
    // // is based on their states and then asking each creature to act.
    // fn interation() {
    //     GList *l = creatures;
    //     GList *next;
    //     Creature *c;
    //     number = 0;
    //     if (prey_program == NULL || pred_program == NULL)
    //         return;
    //     g_list_foreach (creatures, sim_act, NULL);

    //     // purge dead creatures from simulation
    //     while (l != NULL)
    //     {
    //         GList *next = l->next;
    //         c = l->data;
    //         if (c->energy <= 0)
    //         {
    //             g_free (c);
    //             creatures = g_list_delete_link (creatures, l);
    //             number--;
    //         }
    //         l = next;
    //     }
    // }
}
