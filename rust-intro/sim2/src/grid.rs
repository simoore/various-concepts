use crate::location::Location;

/*****************************************************************************/
/*********** TYPES ***********************************************************/
/*****************************************************************************/

pub struct Grid {
    loc: Vec<Vec<Location>>,
    xsize: usize,
    ysize: usize,
}

/*****************************************************************************/
/*********** TRAITS **********************************************************/
/*****************************************************************************/

pub trait HasLocation {
    fn xcoord(&self) -> i32;
    fn ycoord(&self) -> i32;
}

/*****************************************************************************/
/*********** FUNCTIONS *******************************************************/
/*****************************************************************************/

impl Grid {
    /// Creates a new grid for the simulation.
    ///
    /// @param xsize
    ///     The east-west size of the grid.
    /// @param ysize
    ///     The north-south size of the grid.
    /// @returns
    ///     The new grid.
    pub fn new(xsize: usize, ysize: usize) -> Grid {
        let mut loc: Vec<Vec<Location>> = Vec::new();
        for _ in 0..xsize {
            loc.push(vec![Location::new(); ysize])
        }
        Grid { loc, xsize, ysize }
    }

    /// Returns the location object for an object that has coordinates.
    ///
    /// @param hl
    ///     An object with coordinates that can be located on the grid.
    /// @returns
    ///     The location object at the location of the hl object.
    pub fn loc(&self, x: i32, y: i32) -> &Location {
        &self.loc[usize::try_from(x).unwrap()][usize::try_from(y).unwrap()]
    }

    /// Returns the location object for an object that has coordinates.
    ///
    /// @param hl
    ///     An object with coordinates that can be located on the grid.
    /// @returns
    ///     The location object at the location of the hl object.
    pub fn location(&mut self, hl: &impl HasLocation) -> &mut Location {
        let x = hl.xcoord();
        let y = hl.ycoord();
        &mut self.loc[usize::try_from(x).unwrap()][usize::try_from(y).unwrap()]
    }

    /// The east-west size of the grid.
    pub fn xsize(&self) -> usize {
        self.xsize
    }

    /// The north-south size of the grid.
    pub fn ysize(&self) -> usize {
        self.ysize
    }
}
