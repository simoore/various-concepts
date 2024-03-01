use std::collections::HashMap;

/*****************************************************************************/
/********** TYPES ************************************************************/
/*****************************************************************************/

/// Defines the operator used in condition expressions.
enum ConditionOp { Lt, Gt, Eq }

/// Defines the operator used in arithmetic expressions.
enum ArithmeticOp { Add, Sub }

/// The direction an action is performed in.
enum Direction { N, NE, E, SE, S, SW, W, NW }

/// The action that the creature performs after the sim2 program runs.
enum Action { Move(Direction), Hunt(Direction), Breed(Direction), Rest(i32) }

/// The Identifier enum list special identifiers that generate values in different ways compared to normal variables.
enum Identifier { Rand, Energy, DailyRest }

/// A constant value.
struct Constant { value: i32 }

/// A variable that can change value during execution of the sim2 program.
struct Variable { key: String }

/// Represents an expression where two identifiers are combined.
struct Arithmetic {
    a: Box<dyn Expression>,
    b: Box<dyn Expression>,
    op: ArithmeticOp,
}

/// Condiditions represent a boolean expression. sim2 defines true as 1 and false as 0 as expression must evaluate
/// to an integer value.
struct Condition {
    a: Box<dyn Expression>,
    b: Box<dyn Expression>,
    op: ConditionOp,
}

struct Assign {
    var: Variable,
    expression: Box<dyn Expression>,
}

struct IfThen {
    condition: Condition,
    statement_true: Box<dyn Statement>,
    statement_false: Box<dyn Statement>,
}

struct Block {
    statements: Vec<Box<dyn Statement>>,
}

struct Program {
    statement: Box<dyn Statement>,
    vars: HashMap<String, i32>
}

/// When the program is evaluated, we can take into account the state of the creature when making decisions about
/// its behaviour. There are two states that can be considered. Its energy and and how much it has rested during
/// the day.
struct CreatureStats {
    energy: i32,
    daily_rest: i32,
}

/*****************************************************************************/
/********** TRAITS ***********************************************************/
/*****************************************************************************/

/// Expressions return an integer value when executed.
trait Expression {
    fn execute(&self, cs: &CreatureStats, vars: &HashMap<String, i32>) -> i32;
}

/// Statements must evaluate to some Action type as the purpose of a sim2 program is to determine an action 
/// that the predator/prey actor must do.
trait Statement {
    fn execute(&self, cs: &CreatureStats, vars: &mut HashMap<String, i32>) -> Option<Action>;
}

/*****************************************************************************/
/********** FUNCTIONS ********************************************************/
/*****************************************************************************/

impl Expression for Identifier {
    fn execute(&self, cs: &CreatureStats, _vars: &HashMap<String, i32>) -> i32 {
        match self {
            Identifier::Energy => cs.energy,
            Identifier::DailyRest => cs.daily_rest,
            Identifier::Rand => 1, // TODO: generate random number
        }
    }
}

impl Expression for Constant {
    fn execute(&self, _cs: &CreatureStats, _vars: &HashMap<String, i32>) -> i32 {
        self.value
    }
}

impl Expression for Variable {
    fn execute(&self, _cs: &CreatureStats, vars: &HashMap<String, i32>) -> i32 {
        *vars.get(&self.key).unwrap()
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

impl Expression for Condition {
    fn execute(&self, cs: &CreatureStats, vars: &HashMap<String, i32>) -> i32 {
        let istrue = match self.op {
            ConditionOp::Lt => self.a.execute(cs, vars) < self.b.execute(cs, vars),
            ConditionOp::Gt => self.a.execute(cs, vars) > self.b.execute(cs, vars),
            ConditionOp::Eq => self.a.execute(cs, vars) == self.b.execute(cs, vars),
        };
        if istrue { 1 } else { 0 }
    } 
}

impl Statement for Assign {
    fn execute(&self, cs: &CreatureStats, vars: &mut HashMap<String, i32>) -> Option<Action> {
        let value = self.expression.execute(&cs, &vars);
        vars.insert(self.var.key.clone(), value);
        None
    }
}

impl Statement for IfThen {
    fn execute(&self, cs: &CreatureStats, vars: &mut HashMap<String, i32>) -> Option<Action> {
        if self.condition.execute(cs, &vars) == 1 {
            self.statement_true.execute(cs, vars)
        } else {
            self.statement_false.execute(cs, vars)
        }
    }
}

impl Statement for Block {
    fn execute(&self, cs: &CreatureStats, vars: &mut HashMap<String, i32>) -> Option<Action> {
        let mut action: std::option::Option<Action> = None;
        for statement in &self.statements {
            action = statement.execute(&cs, vars);
        }
        action
    }
}

impl Program {
    fn execute(&mut self, energy: i32, daily_rest: i32) -> Action {
        let cs = CreatureStats { energy, daily_rest };
        self.statement.execute(&cs, &mut self.vars).unwrap()
    }
}