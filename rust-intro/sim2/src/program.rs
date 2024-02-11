

pub trait Statement {
    /// The type returned in the event of a conversion error.
    #[stable(feature = "try_from", since = "1.34.0")]
    type Error;

    /// Performs the conversion.
    #[stable(feature = "try_from", since = "1.34.0")]
    #[rustc_diagnostic_item = "try_from_fn"]
    fn try_from(value: T) -> Result<Self, Self::Error>;
}

trait expression {
    fn evaluate(&self) -> i32;
}

pub trait Expression {

}

#ifndef _PROGRAM_H_
#define _PROGRAM_H_

#include <glib.h>

typedef enum {IDENT, COND, ARITH} ExpressionType; 
typedef enum {BLOCK, IF_THEN, ASSIGN, ACTION} StatementType;
typedef enum {MOVE, HUNT, BREED, REST, NONE} ActionType;
typedef enum {ADD, SUB, LT, GT, EQ} Operator;
typedef enum {VAR, INT, RAND, ENERGY, DAILY_REST} IdentifierType;
typedef enum {N, NE, E, SE, S, SW, W, NW, STILL} Direction;

typedef struct Statement_ Statement;

typedef struct {
    IdentifierType type;
    int value;
} Identifier;

typedef struct {
    Identifier *identifier_left;
    Identifier *identifier_right;
    Operator op;
} Arithmetic;

typedef struct {
    Identifier *identifier_left;
    Identifier *identifier_right;
    Operator op;
} Condition;

typedef struct {
    ExpressionType type;
    union {
        Condition *condition;
        Identifier *identifier;
        Arithmetic *arithmetic;
    } op;
} Expression;

typedef struct {
    ActionType action;
    union {
        int time;
        Direction direction;
    } op;
} Action;

typedef struct {
    Identifier *var;
    Expression *expression;
} Assign;

typedef struct {
    Condition *condition;
    Statement *statement_true;
    Statement *statement_false;
} IfThen;

typedef struct {
    GList *statements;
} Block;

struct Statement_ {
    StatementType type;
    union { 
        Block *block;
        IfThen *if_then;
        Assign *assign;
        Action *action;
    } op;
};

typedef struct {
    Statement *statement;
    GHashTable *variables;
} Program;

Program *
program_new (Statement *statement, GHashTable *vars);

void 
program_free (Program *program);

void 
program_free_statement_void (gpointer data);

Statement *
program_new_statement (StatementType type, void *op);

void 
program_free_statement (Statement *statement);

Statement *
program_new_block (GList *list);

void 
program_free_block (Block *block);

Statement *
program_new_if_then (Condition *condition, Statement *true_, Statement *false_);
    
void 
program_free_if_then (IfThen *if_then);

Statement *
program_new_assign (Identifier *variable, Expression *expression);

void 
program_free_assign (Assign *assign);

Statement *
program_new_action_with_direction (ActionType type, Direction direction);

Statement *
program_new_action_with_time (int time);

void 
program_free_action (Action *action);

Expression *
program_new_expression (ExpressionType type, void *op);

void 
program_free_expression (Expression *expression);

void 
program_free_expression_void (gpointer data);

Expression *
program_new_arithmetic (Operator op, Identifier *ident_a, Identifier *ident_b);

void 
program_free_arithmetic (Arithmetic *arithmetic);

Condition *
program_new_condition (Operator oper, Identifier *ident_a, Identifier *ident_b);

void 
program_free_condition (Condition *condition);

Identifier *
program_new_identifier (IdentifierType type, int value);

void 
program_free_identifier (Identifier *identifier);

Action *
program_execute (Program *program, int energy_, int daily_rest_);

void 
program_print (Program *program);

#endif /* _PROGRAM_H_ */