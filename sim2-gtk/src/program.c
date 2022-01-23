#include <glib.h>
#include "program.h"

static int energy = 0;
static int daily_rest = 0;

static Action *
program_execute_statement (Statement *statement);

static void 
program_print_statement (Statement *statement, int indents);

////////////////////////////////////////////////////////////////////////////////
//////////////// Statement Creation and Destruction Functions //////////////////
////////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
* Creates a new program for a top level statement. Call program_free () to 
* deallocate the program.
*
* statement     The top level statement of the program.
* return        The program containing the top level statement.
*******************************************************************************/
Program *
program_new (Statement *statement, GHashTable *vars)
{
    Program *program = g_new (Program, 1);
    program->statement = statement;
    program->variables = vars;
    return program;
}

/*******************************************************************************
* Frees memory associated with a program structure.
*
* program   The program to dellocate memory from.
*******************************************************************************/
void
program_free (Program *program)
{
    if (program == NULL)
        return;
    program_free_statement (program->statement);
    g_hash_table_destroy (program->variables);
    g_free (program);
}

Statement *
program_new_statement (StatementType type, void *op)
{
    Statement *statement = g_new (Statement, 1);
    statement->type = type;
    switch (type)
    {
        case BLOCK:   statement->op.block = op;   break;
        case IF_THEN: statement->op.if_then = op; break;
        case ASSIGN:  statement->op.assign = op;  break;
        case ACTION:  statement->op.action = op;  break;
    }
    return statement;
}

void
program_free_statement (Statement *statement)
{
    switch (statement->type)
    {
        case BLOCK:   program_free_block (statement->op.block);   break;
        case IF_THEN: program_free_if_then (statement->op.if_then); break;
        case ASSIGN:  program_free_assign (statement->op.assign);  break;
        case ACTION:  program_free_action (statement->op.action);  break;
    }
    g_free (statement);
}

void 
program_free_statement_void (gpointer data)
{
    program_free_statement (data);
}

/*******************************************************************************
* 
*******************************************************************************/
Statement *
program_new_block (GList *list)
{
    Block *block = g_new (Block, 1);    
    block->statements = list;
    return program_new_statement (BLOCK, block);
}

void
program_free_block (Block *block)
{
    g_list_free_full (block->statements, program_free_statement_void);
    g_free (block);
}

/*******************************************************************************
* Creates an IfThen statement. Cakk program_if_then_free () to deallocate.
*
* condition         The condition to evaluate in the IfThen statement.
* true_             The statement to execute on a non 0 condition.
* false_            The statement to execute on a 0 condition
*******************************************************************************/
Statement *
program_new_if_then (Condition *condition, Statement *true_, Statement *false_)
{
    IfThen *if_then = g_new (IfThen, 1);
    if_then->condition = condition;
    if_then->statement_true = true_;
    if_then->statement_false = false_;
    return program_new_statement (IF_THEN, if_then);
}

void
program_free_if_then (IfThen *if_then)
{
    program_free_condition (if_then->condition);
    program_free_statement (if_then->statement_true);
    if (if_then->statement_false != NULL)
        program_free_statement (if_then->statement_false);
    g_free (if_then);
}

Statement *
program_new_assign (Identifier *variable, Expression *expression)
{
    Assign *assign = g_new (Assign, 1);
    assign->var = variable;
    assign->expression = expression;
    return program_new_statement (ASSIGN, assign);
}

void
program_free_assign (Assign *assign)
{
    program_free_identifier (assign->var);
    program_free_expression (assign->expression);
    g_free (assign);
}

/*******************************************************************************
*
*******************************************************************************/
Statement *
program_new_action_with_direction (ActionType type, Direction direction)
{
    Action *action = g_new (Action, 1);
    action->action = type;
    action->op.direction = direction;
    return program_new_statement (ACTION, action);
}

/*******************************************************************************
* 
*******************************************************************************/
Statement *
program_new_action_with_time (int time)
{
    Action *action = g_new (Action, 1);
    action->action = REST;
    action->op.time = time;
    return program_new_statement (ACTION, action);
}

/*******************************************************************************
* Frees the memory allocation associated with an action. Action data structures
* have no dynamically allocated components.
*
* Action        The action to be dellocated.
*******************************************************************************/
void 
program_free_action (Action *action)
{
    g_free (action);
}

////////////////////////////////////////////////////////////////////////////////
///////////////////// Expression Creation and Destruction //////////////////////
////////////////////////////////////////////////////////////////////////////////

Expression *
program_new_expression (ExpressionType type, void *op)
{
    Expression *expression = g_new (Expression, 1);
    expression->type = type;
    switch (type)
    {
        case IDENT: expression->op.identifier = op;
        case COND:  expression->op.condition = op;
        case ARITH: expression->op.arithmetic = op;
    }
}

void
program_free_expression (Expression *expression)
{
    switch (expression->type)
    {
        case IDENT: program_free_identifier (expression->op.identifier);
        case COND:  program_free_condition (expression->op.condition);
        case ARITH: program_free_arithmetic (expression->op.arithmetic);
    }
    g_free (expression);
}

void
program_free_expression_void (gpointer data)
{
    program_free_expression (data);
}

Expression *
program_new_arithmetic (Operator oper, Identifier *ident_a, Identifier *ident_b)
{
    Arithmetic *arithmetic = g_new (Arithmetic, 1);
    arithmetic->identifier_left = ident_a;
    arithmetic->identifier_right = ident_b;
    arithmetic->op = oper;
    return program_new_expression (ARITH, arithmetic);
}

void
program_free_arithmetic (Arithmetic *arithmetic)
{
    program_free_identifier (arithmetic->identifier_left);
    program_free_identifier (arithmetic->identifier_right);
    g_free (arithmetic);
}

Condition *
program_new_condition (Operator oper, Identifier *ident_a, Identifier *ident_b)
{
    Condition *condition = g_new (Condition, 1);
    condition->identifier_left = ident_a;
    condition->identifier_right = ident_b;
    condition->op = oper;
    return condition;
}

void
program_free_condition (Condition *condition)
{
    program_free_identifier (condition->identifier_left);
    program_free_identifier (condition->identifier_right);
    g_free (condition);
}

Identifier *
program_new_identifier (IdentifierType type, int value)
{
    Identifier *identifier = g_new (Identifier, 1);
    identifier->type = type;
    identifier->value = value;
    return identifier;
}

void
program_free_identifier (Identifier *identifier)
{
    if (identifier->type != VAR)
        g_free (identifier);
}

////////////////////////////////////////////////////////////////////////////////
//////////////////// Program Functionality Functions ///////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
*
*******************************************************************************/
static int
program_execute_identifier (Identifier *identifier)
{
    GRand *random;
    int number;

    switch (identifier->type)
    {
        case INT:
            return identifier->value;
        case VAR:
            return identifier->value;
        case ENERGY:
            return energy;
        case DAILY_REST:
            return daily_rest;
        case RAND:
            random = g_rand_new ();
            number = g_rand_int_range (random, 0, 1000);
            g_rand_free (random);
            return number;
    } 
}

/*******************************************************************************
* Determines the value of an arithmetic expression. These expressions add or
* substract two identifiers.
*
* arithmetic        The arithmetic expression.
* return            Its value.
*******************************************************************************/
static int
program_execute_arithmetic (Arithmetic *arithmetic)
{
    int value_left, value_right;  
    
    value_left = program_execute_identifier (arithmetic->identifier_left);
    value_right = program_execute_identifier (arithmetic->identifier_right);

    switch (arithmetic->op)
    {
        case ADD:
            return value_left + value_right;
        case SUB:
            return value_left - value_right;
    }
}

/*******************************************************************************
*
*******************************************************************************/
static int
program_execute_condition (Condition *condition)
{
    int value_left, value_right;

    value_left = program_execute_identifier (condition->identifier_left);
    value_right = program_execute_identifier (condition->identifier_right);

    switch (condition->op)
    {
        case LT: 
            return (value_left < value_right);
        case GT:
            return (value_left > value_right);
        case EQ:
            return (value_left == value_right);
    }
}

/*******************************************************************************
* Expressions return integer values. There are many types of expressions and 
* function determines the type than evalues the value.
*
* expression        The expression to evaluate.
* return            The value of the expression.
*******************************************************************************/
static int
program_execute_expression (Expression *expression)
{
    switch (expression->type)
    {
        case IDENT:
            return program_execute_identifier (expression->op.identifier);
        case COND:
            return program_execute_condition (expression->op.condition);
        case ARITH:
            return program_execute_arithmetic (expression->op.arithmetic);
    }
}

/*******************************************************************************
* Executes an assignment statement. This changes the value of a variable to an
* evaluated expression. The return action is always NULL.
*
* assign        The assignment statement.
* return        The action of the statement which is always NULL. 
*******************************************************************************/
static Action *
program_execute_assign (Assign *assign)
{
    int value = program_execute_expression (assign->expression);
    assign->var->value = value;
    return NULL;
}

/*******************************************************************************
* Executes an if statement. It evaluates a condition expression and executes
* a true or false statement. The false statement can be NULL.
*
* if_then       The IfThen statement to be executed.
* return        The action of the executed statement, possibly NULL.
*******************************************************************************/
static Action *
program_execute_if_then (IfThen *if_then)
{
    int value = program_execute_condition (if_then->condition);
    if (value)
        return program_execute_statement (if_then->statement_true);
    else
        if (if_then->statement_false != NULL)
            return program_execute_statement (if_then->statement_false);
    return NULL;
}

/*******************************************************************************
* Iterates through a list and executes each statement. The final action will be
* returned.
*
* block     The block to be executed.
* return    The action of the last statement in the block.
*******************************************************************************/
static Action *
program_execute_block (Block *block)
{
    GList *l;
    Action *action;
    for (l = block->statements; l != NULL; l = l->next)
        action = program_execute_statement ((Statement *)l);
    return action;
}

/*******************************************************************************
* A statement can be of many types. This function checks the type and executes
* the appropiate function. All statements should return an agent action. This 
* can be NULL.
*
* statement     The statement to be executed.
* return        The last executed agent action, can be NULL;
*******************************************************************************/
static Action *
program_execute_statement (Statement *statement)
{
    switch (statement->type)
    {
        case BLOCK: 
            return program_execute_block (statement->op.block); 
        case IF_THEN: 
            return program_execute_if_then (statement->op.if_then); 
        case ASSIGN:
            return program_execute_assign (statement->op.assign);
        case ACTION:
            return statement->op.action;
        default:
            return NULL;
    }
}

/*******************************************************************************
* Top level execution of a program. Returns an action to the caller with which
* the creatures behaviour is changed.
*
* program       The program to execute.
* energy_       The energy of the calling creature to influene its behaviour.
* daily_rest_   The daily_rest of the calling creature.
* return        The action for the creature. Can be null;
*******************************************************************************/
Action *
program_execute (Program *program, int energy_, int daily_rest_)
{
    energy = energy_;
    daily_rest = daily_rest_;
    return program_execute_statement (program->statement);   
}

////////////////////////////////////////////////////////////////////////////////
///////////////////// Program Printing Functions ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static void 
program_print_indents (int num)
{
    int i;
    for (i = 0; i < num; i++)
        g_print ("    ");
}

static void 
program_print_identifier (Identifier *identifier)
{
    switch (identifier->type)
    {
        case VAR:           g_print ("VAR"); break;
        case INT:           g_print ("%d", identifier->value); break;
        case RAND:          g_print ("rand"); break;
        case ENERGY:        g_print ("energy"); break;
        case DAILY_REST:    g_print ("daily_rest"); break;
    }
}

static void 
program_print_arithmetic (Arithmetic *arithmetic)
{
    switch (arithmetic->op)
    {
        case ADD: g_print ("add "); break;
        case SUB: g_print ("sub "); break;
    }
    program_print_identifier (arithmetic->identifier_left);
    program_print_identifier (arithmetic->identifier_right);
}

static void 
program_print_condition (Condition *condition)
{
    program_print_identifier (condition->identifier_left);
    switch (condition->op)
    {
        case LT: g_print (" < "); break;
        case GT: g_print (" > "); break;
        case EQ: g_print (" == "); break;
    }
    program_print_identifier (condition->identifier_right);
}

static void 
program_print_expression (Expression *expression)
{
    switch (expression->type)
    {
        case COND:  
            program_print_condition (expression->op.condition); break;
        case ARITH: 
            program_print_arithmetic (expression->op.arithmetic); break;
        case IDENT: 
            program_print_identifier (expression->op.identifier); break;
    }
}

static void 
program_print_block (Block *block, int indents)
{
    GList *l;
    g_print ("block\n");
    for (l = block->statements; l != NULL; l = l->next)
        program_print_statement (l->data, indents + 1);
    program_print_indents (indents);
    g_print ("end\n");
}

static void 
program_print_if_then (IfThen *if_then, int indents)
{
    g_print ("if ");
    program_print_condition (if_then->condition);
    g_print (" then\n");
    program_print_statement (if_then->statement_true, indents + 1);
    if (if_then->statement_false != NULL)
    {
        program_print_indents (indents);
        g_print ("else\n");
        program_print_statement (if_then->statement_false, indents + 1);
    }
    program_print_indents (indents);
    g_print ("end\n");
}

static void 
program_print_assign (Assign *assign, int indents)
{
    g_print ("VAR");
    g_print (" = ");
    program_print_expression (assign->expression);
    g_print ("\n");
}

static void 
program_print_action (Action *action, int indents)
{
    switch (action->action)
    {
        case MOVE:  g_print ("move ");  break;
        case REST:  g_print ("rest ");  break;
        case BREED: g_print ("breed "); break;
        case HUNT:  g_print ("hunt ");  break;
    }
    
    if (action->action == REST)
    {
        g_print ("%d\n", action->op.time);
        return;
    }
    
    switch (action->op.direction)
    {
        case N:  g_print ("N");     break;
        case NE: g_print ("NE");    break;
        case E:  g_print ("E");     break;
        case SE: g_print ("SE");    break;
        case S:  g_print ("S");     break;
        case SW: g_print ("SW");    break;
        case W:  g_print ("W");     break;
        case NW: g_print ("NW");    break;
    }
    g_print ("\n");
}

static void 
program_print_statement (Statement *statement, int indents)
{
    program_print_indents (indents);
    switch (statement->type)
    {
        case BLOCK:   
            program_print_block (statement->op.block, indents); break;
        case IF_THEN: 
            program_print_if_then (statement->op.if_then, indents); break;
        case ASSIGN:  
            program_print_assign (statement->op.assign, indents); break;
        case ACTION:  
            program_print_action (statement->op.action, indents); break;
    }
}

void 
program_print (Program *program)
{
    if (program == NULL)
    {
        g_print ("program is null\n");
        return;
    }
    g_print ("sim\n");    
    program_print_statement (program->statement, 1);
    g_print ("end\n");
}

