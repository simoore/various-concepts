
trait Expression {
    fn execute(&self) -> i32;
}

trait Statement {
    fn execute(&self, energy: i32, daily_rest: i32) -> Action;
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


struct Program {
    statement: &Statement,
}

struct Block {

}

impl Statement for Program {
    fn execute(&self, energy: i32, daily_rest: i32) -> i32 {
        return self.execute(energy, daily_rest);
    }
}

Action *
program_execute (Program *program, int energy_, int daily_rest_);

void 
program_print (Program *program);

#endif /* _PROGRAM_H_ */