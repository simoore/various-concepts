use std::collections::HashMap;
use std::fmt::{Debug, Display};

use rand::Rng;

/*****************************************************************************/
/********** TRAITS ***********************************************************/
/*****************************************************************************/

/// Expressions return an integer value when executed.
pub trait Expression: Debug + Display {
    fn execute(&self, cs: &CreatureStats, vars: &HashMap<String, i32>) -> i32;
}

/// Statements must evaluate to some Action type as the purpose of a sim2 program is to determine an action
/// that the predator/prey actor must do.
pub trait Statement: Debug + Display {
    fn execute(&self, cs: &CreatureStats, vars: &mut HashMap<String, i32>) -> Action;
}

/*****************************************************************************/
/********** TYPES ************************************************************/
/*****************************************************************************/

/// Defines the operator used in condition expressions.
#[derive(Debug)]
pub enum ConditionOp {
    Lt,
    Gt,
    Eq,
}

impl Display for ConditionOp {
    fn fmt(&self, fmt: &mut std::fmt::Formatter) -> std::fmt::Result {
        match &self {
            ConditionOp::Lt => write!(fmt, "<"),
            ConditionOp::Gt => write!(fmt, ">"),
            ConditionOp::Eq => write!(fmt, "=="),
        }
    }
}

/// Defines the operator used in arithmetic expressions.
#[derive(Debug)]
pub enum ArithmeticOp {
    Add,
    Sub,
}

impl Display for ArithmeticOp {
    fn fmt(&self, fmt: &mut std::fmt::Formatter) -> std::fmt::Result {
        match &self {
            ArithmeticOp::Add => write!(fmt, "Add"),
            ArithmeticOp::Sub => write!(fmt, "Sub"),
        }
    }
}

/// When the program is evaluated, we can take into account the state of the creature when making decisions about
/// its behaviour. There are two states that can be considered. Its energy and and how much it has rested during
/// the day.
pub struct CreatureStats {
    energy: i32,
    daily_rest: i32,
}

/*****************************************************************************/
/********** TYPE: Direction **************************************************/
/*****************************************************************************/

/// The direction an action is performed in.
#[derive(Clone, Debug, PartialEq)]
pub enum Direction {
    N,
    NE,
    E,
    SE,
    S,
    SW,
    W,
    NW,
}

impl Display for Direction {
    fn fmt(&self, fmt: &mut std::fmt::Formatter) -> std::fmt::Result {
        match &self {
            Direction::N => write!(fmt, "N"),
            Direction::NE => write!(fmt, "NE"),
            Direction::E => write!(fmt, "E"),
            Direction::SE => write!(fmt, "SE"),
            Direction::S => write!(fmt, "S"),
            Direction::SW => write!(fmt, "SW"),
            Direction::W => write!(fmt, "W"),
            Direction::NW => write!(fmt, "NW"),
        }
    }
}

/*****************************************************************************/
/********** TYPE: Action *****************************************************/
/*****************************************************************************/

/// The action that the creature performs after the sim2 program runs.
#[derive(Clone, Debug, PartialEq)]
pub enum Action {
    Move(Direction),
    Hunt(Direction),
    Breed(Direction),
    Rest(i32),
    None,
}

impl Display for Action {
    fn fmt(&self, fmt: &mut std::fmt::Formatter) -> std::fmt::Result {
        match &self {
            Action::Move(dir) => write!(fmt, "Move({})", dir),
            Action::Hunt(dir) => write!(fmt, "Hunt({})", dir),
            Action::Breed(dir) => write!(fmt, "Breed({})", dir),
            Action::Rest(count) => write!(fmt, "Rest({})", count),
            Action::None => write!(fmt, "None"),
        }
    }
}

impl Statement for Action {
    fn execute(&self, _cs: &CreatureStats, _vars: &mut HashMap<String, i32>) -> Action {
        self.clone()
    }
}

/*****************************************************************************/
/********** TYPE: Special Identifier *****************************************/
/*****************************************************************************/

/// The Identifier enum list special identifiers that generate values in different ways compared to normal variables.
#[derive(Debug)]
pub enum SpecialIdentifier {
    Rand,
    Energy,
    DailyRest,
}

impl Display for SpecialIdentifier {
    fn fmt(&self, fmt: &mut std::fmt::Formatter) -> std::fmt::Result {
        match &self {
            SpecialIdentifier::DailyRest => write!(fmt, "DailyRest"),
            SpecialIdentifier::Energy => write!(fmt, "Energy"),
            SpecialIdentifier::Rand => write!(fmt, "Rand"),
        }
    }
}

impl Expression for SpecialIdentifier {
    fn execute(&self, cs: &CreatureStats, _vars: &HashMap<String, i32>) -> i32 {
        match self {
            SpecialIdentifier::Energy => cs.energy,
            SpecialIdentifier::DailyRest => cs.daily_rest,
            SpecialIdentifier::Rand => {
                let mut rng = rand::thread_rng();
                rng.gen_range(0..1000)
            }
        }
    }
}

/*****************************************************************************/
/********** TYPE: Constant ***************************************************/
/*****************************************************************************/

/// A constant value.
#[derive(Debug)]
pub struct Constant {
    value: i32,
}

impl Constant {
    pub fn new(value: i32) -> Constant {
        Constant { value }
    }
}

impl Display for Constant {
    fn fmt(&self, fmt: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(fmt, "{}", self.value)
    }
}

impl Expression for Constant {
    fn execute(&self, _cs: &CreatureStats, _vars: &HashMap<String, i32>) -> i32 {
        self.value
    }
}

/*****************************************************************************/
/********** TYPE: Variable ***************************************************/
/*****************************************************************************/

/// A variable that can change value during execution of the sim2 program.
#[derive(Debug)]
pub struct Variable {
    key: String,
}

impl Variable {
    pub fn new(key: String) -> Variable {
        Variable { key }
    }
}

impl Display for Variable {
    fn fmt(&self, fmt: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(fmt, "{}", self.key)
    }
}

impl Expression for Variable {
    fn execute(&self, _cs: &CreatureStats, vars: &HashMap<String, i32>) -> i32 {
        *vars.get(&self.key).unwrap()
    }
}

/*****************************************************************************/
/********** TYPE: Identifier *************************************************/
/*****************************************************************************/

#[derive(Debug)]
pub enum Identifier {
    Special(SpecialIdentifier),
    Variable(Variable),
    Constant(Constant),
}

impl Identifier {
    pub fn new_specialized(ident: SpecialIdentifier) -> Identifier {
        Identifier::Special(ident)
    }

    pub fn new_variable(key: String) -> Identifier {
        Identifier::Variable(Variable::new(key))
    }

    pub fn new_constant(val: i32) -> Identifier {
        Identifier::Constant(Constant::new(val))
    }
}

impl Display for Identifier {
    fn fmt(&self, fmt: &mut std::fmt::Formatter) -> std::fmt::Result {
        match self {
            Identifier::Special(s) => write!(fmt, "{}", s),
            Identifier::Variable(v) => write!(fmt, "{}", v),
            Identifier::Constant(c) => write!(fmt, "{}", c),
        }
    }
}

impl Expression for Identifier {
    fn execute(&self, cs: &CreatureStats, vars: &HashMap<String, i32>) -> i32 {
        match self {
            Identifier::Special(s) => s.execute(cs, vars),
            Identifier::Variable(v) => v.execute(cs, vars),
            Identifier::Constant(c) => c.execute(cs, vars),
        }
    }
}

/*****************************************************************************/
/********** TYPE: Arithmetic *************************************************/
/*****************************************************************************/

// Represents an expression where two identifiers are combined.
#[derive(Debug)]
pub struct Arithmetic {
    a: Identifier,
    b: Identifier,
    op: ArithmeticOp,
}

impl Arithmetic {
    pub fn new(a: Identifier, b: Identifier, op: ArithmeticOp) -> Arithmetic {
        Arithmetic { a, b, op }
    }
}

impl Display for Arithmetic {
    fn fmt(&self, fmt: &mut std::fmt::Formatter) -> std::fmt::Result {
        // TODO: Fix this fmt function to display arithmic correctly.
        write!(fmt, "Action")
    }
}

impl Expression for Arithmetic {
    fn execute(&self, cs: &CreatureStats, vars: &HashMap<String, i32>) -> i32 {
        match self.op {
            ArithmeticOp::Add => self.a.execute(cs, vars) + self.b.execute(cs, vars),
            ArithmeticOp::Sub => self.a.execute(cs, vars) - self.b.execute(cs, vars),
        }
    }
}

/*****************************************************************************/
/********** TYPE: Condition **************************************************/
/*****************************************************************************/

/// Condiditions represent a boolean expression. sim2 defines true as 1 and false as 0 as expression must evaluate
/// to an integer value.
#[derive(Debug)]
pub struct Condition {
    a: Identifier,
    b: Identifier,
    op: ConditionOp,
}

impl Condition {
    pub fn new(a: Identifier, b: Identifier, op: ConditionOp) -> Condition {
        Condition { a, b, op }
    }
}

impl Display for Condition {
    fn fmt(&self, fmt: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(fmt, "( {} {} {} )", self.a, self.op, self.b)
    }
}

impl Expression for Condition {
    fn execute(&self, cs: &CreatureStats, vars: &HashMap<String, i32>) -> i32 {
        let istrue = match self.op {
            ConditionOp::Lt => self.a.execute(cs, vars) < self.b.execute(cs, vars),
            ConditionOp::Gt => self.a.execute(cs, vars) > self.b.execute(cs, vars),
            ConditionOp::Eq => self.a.execute(cs, vars) == self.b.execute(cs, vars),
        };
        if istrue {
            1
        } else {
            0
        }
    }
}

/*****************************************************************************/
/********** TYPE: Assign *****************************************************/
/*****************************************************************************/

#[derive(Debug)]
pub struct Assign {
    var: Variable,
    expression: Box<dyn Expression>,
}

impl Assign {
    pub fn new(var: Variable, expression: Box<dyn Expression>) -> Assign {
        Assign { var, expression }
    }
}

impl Display for Assign {
    fn fmt(&self, fmt: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(fmt, "{} = {}", self.var, self.expression)
    }
}

impl Statement for Assign {
    fn execute(&self, cs: &CreatureStats, vars: &mut HashMap<String, i32>) -> Action {
        let value = self.expression.execute(cs, vars);
        vars.insert(self.var.key.clone(), value);
        Action::None
    }
}

/*****************************************************************************/
/********** TYPE: IfThen *****************************************************/
/*****************************************************************************/

#[derive(Debug)]
pub struct IfThen {
    condition: Condition,
    then_statement: Box<dyn Statement>,
    else_statement: Option<Box<dyn Statement>>,
}

impl IfThen {
    pub fn new(
        condition: Condition,
        then_statement: Box<dyn Statement>,
        else_statement: Option<Box<dyn Statement>>,
    ) -> IfThen {
        IfThen {
            condition,
            then_statement,
            else_statement,
        }
    }
}

impl Display for IfThen {
    fn fmt(&self, fmt: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(fmt, "if {} then ", self.condition)?;
        write!(fmt, "{} ", self.then_statement)?;
        if let Some(else_statement) = &self.else_statement {
            write!(fmt, "else ")?;
            write!(fmt, "{} ", else_statement)?;
        }
        write!(fmt, "end")?;
        Ok(())
    }
}

impl Statement for IfThen {
    fn execute(&self, cs: &CreatureStats, vars: &mut HashMap<String, i32>) -> Action {
        if self.condition.execute(cs, vars) == 1 {
            self.then_statement.execute(cs, vars)
        } else {
            match &self.else_statement {
                Some(statement) => statement.execute(cs, vars),
                None => Action::None,
            }
        }
    }
}

/*****************************************************************************/
/********** TYPE: Block ******************************************************/
/*****************************************************************************/

#[derive(Debug)]
pub struct Block {
    statements: Vec<Box<dyn Statement>>,
}

impl Block {
    pub fn new(statements: Vec<Box<dyn Statement>>) -> Block {
        Block { statements }
    }
}

impl Display for Block {
    fn fmt(&self, fmt: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(fmt, "Block ")?;
        for s in &self.statements {
            write!(fmt, "{} ", s)?;
        }
        write!(fmt, "End")?;
        Ok(())
    }
}

impl Statement for Block {
    fn execute(&self, cs: &CreatureStats, vars: &mut HashMap<String, i32>) -> Action {
        let mut action = Action::None;
        for statement in &self.statements {
            action = statement.execute(cs, vars);
        }
        action
    }
}

/*****************************************************************************/
/********** TYPE: Program ****************************************************/
/*****************************************************************************/

#[derive(Debug)]
pub struct Program {
    statement: Box<dyn Statement>,
}

impl Program {
    pub fn new(statement: Box<dyn Statement>) -> Program {
        Program { statement }
    }

    pub fn execute(&self, energy: i32, daily_rest: i32) -> Action {
        let cs = CreatureStats { energy, daily_rest };
        // TODO: handle variable hash table properly in this module (it isn't being used at the moment).
        let mut vars = HashMap::<String, i32>::new();
        self.statement.execute(&cs, &mut vars)
    }
}
