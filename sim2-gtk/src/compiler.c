#include <gio/gio.h>
#include "lexer.h"
#include "program.h"

static Identifier *
compiler_get_identifier (Token token, GHashTable *vars);

static Statement *
compiler_get_statement (Token token, GHashTable *vars);

/*******************************************************************************
* Initializes and fills the table of variables.
*
* vars      The hash table to save all discovered variables.
*******************************************************************************/
static void 
compiler_find_variables (GHashTable *vars)
{
    const char *name;
    Token token;
    Identifier *variable;
    
    token = lexer_get_token ();
    while (token != EOFT)
    {
        if (token == IDT)
        {
            name = lexer_get_string_value ();
            g_print ("%s\n", name);
            variable = program_new_identifier (VAR, 0);
            g_hash_table_insert (vars, g_strdup (name), variable);
        }
        token = lexer_get_token ();
    }
}

/*******************************************************************************
* TODO: have this call a function pointer to print string to some output on 
* error. Merge with next function.
*******************************************************************************/
const gchar *
compiler_get_error_message ()
{
    return NULL;
}

/*******************************************************************************
* Sets error message and uninitialises the lexer.
*
* error     The error message.
*******************************************************************************/
static void
compiler_error (const char *error)
{
    int line = lexer_get_line ();
    lexer_uninit ();
    g_print ("%d: %s", line, error);
}

/*******************************************************************************
* Constructs a new condition expression. The left parenthese has already been 
* found.
*
* first_token   Convience parameter to check left parenthese has actually been
*               found.
* vars          The table of program variables.
* return        The new condition. Null if error.
*******************************************************************************/
static Condition *
compiler_get_condition (Token first_token, GHashTable *vars)
{
    Identifier *ident_a;
    Identifier *ident_b;
    Operator operator;
    Token token;
    
    if (first_token != LPART)
        return NULL;

    ident_a = compiler_get_identifier (lexer_get_token (), vars);
    if (ident_a == NULL)
        return NULL;

    token = lexer_get_token ();
    switch (token)
    {
        case LTT: operator = LT;    break;
        case GTT: operator = GT;    break;
        case TWOEQT: operator = EQ; break;
        default : 
            compiler_error ("condition operator error\n");
            program_free_identifier (ident_a);
            return NULL;
    }

    token = lexer_get_token ();
    ident_b = compiler_get_identifier (token, vars);
    if (ident_b == NULL)
    {
        program_free_identifier (ident_a);
        return NULL;
    }

    token = lexer_get_token ();
    if (token != RPART)
    {
        compiler_error ("missing RPART\n");
        program_free_identifier (ident_a);
        program_free_identifier (ident_b);
        return NULL;
    }

    return program_new_condition (operator, ident_a, ident_b);
}

/*******************************************************************************
* Constructs an arithmetic expression from the lexer stream. The first token
* has already been discovered and is passed to the function in the operator
* parameter.
*
* operator      Whether the arithmetic operation is add or sub.
* vars          The list of program variables.
* return        An arithmetic operation. NULL if error.
*******************************************************************************/
static Expression *
compiler_get_arithmetic (Operator operator, GHashTable *vars)
{
    Identifier *ident_a;
    Identifier *ident_b;
    
    ident_a = compiler_get_identifier (lexer_get_token (), vars);
    if (ident_a == NULL)
        return NULL;

    ident_b = compiler_get_identifier (lexer_get_token (), vars);
    if (ident_b == NULL)
    {
        program_free_identifier (ident_a);
        return NULL;
    }

    return program_new_arithmetic (operator, ident_a, ident_b); 
}

/*******************************************************************************
* Constructs and identifier with the token already discovered.
*
* token     The token of the apparent identifer.
* vars      The list of progam variables.
* return    The identifier. NULL if error;
*******************************************************************************/
static Identifier *
compiler_get_identifier (Token token, GHashTable *vars)
{
    const char *name;
    int value;

    switch (token)
    {
        case IDT:   
            name = lexer_get_string_value ();
            return g_hash_table_lookup (vars, name);            
        case INTT:  
            value = lexer_get_int_value ();
            return program_new_identifier (INT, value);
        case IDENERGYT:
            return program_new_identifier (ENERGY, 0);
        case IDRESTT:
            return program_new_identifier (DAILY_REST, 0);
        case RANDT: 
            return program_new_identifier (RAND, 0);
    }
    compiler_error ("identifier expected\n");
    return NULL;
}

/*******************************************************************************
* Looks for an expression next in the lexer stream.
*
* vars      The table of program variables.
* return    The expression. NULL if error.
*******************************************************************************/
static Expression *
compiler_get_expression (GHashTable *vars)
{
    Condition *condition;
    Identifier *identifier;
    Token token = lexer_get_token ();

    switch (token)
    {
        case IDT:   identifier = compiler_get_identifier (IDT, vars); break;
        case INTT:  identifier = compiler_get_identifier (INTT, vars);  break;
        case RANDT: identifier = compiler_get_identifier (RANDT, vars); break;
        case LPART: 
            condition = compiler_get_condition (LPART, vars);
            return program_new_expression (COND, condition);
        case ADDT:  return compiler_get_arithmetic (ADD, vars);
        case SUBT:  return compiler_get_arithmetic (SUB, vars);
        default :   
            compiler_error ("expression expected\n");
            return NULL;
    }
    if (identifier == NULL)
        return NULL;
    return program_new_expression (IDENT, identifier);
}

/*******************************************************************************
* Returns an agent action. The first token has already been discoved and this
* function checks the arguments are ok and creates the structure.
*
* first_token   The already discovered first token indicating the statement 
*                   type.
* return        The agent action statement.
*******************************************************************************/
static Statement *
compiler_get_action (Token first_token)
{
    Token token;
    int time;
    Direction dir;

    token = lexer_get_token ();
    if (first_token == RESTT && token != INTT)
        return NULL;
    else if (first_token != RESTT && token < NT)
        return NULL;

    if (token == INTT)
        time = lexer_get_int_value ();
    else
        switch (token)
        {
            case NT: dir = N; break;
            case NET: dir = NE; break;
            case ET: dir = E; break;
            case SET: dir = SE; break;
            case ST: dir = S; break;
            case SWT: dir = SW; break;
            case WT: dir = W; break;
            case NWT: dir = NW; break;
            default: dir = STILL; break;			
        }

    switch (first_token)
    {
        case HUNTT:  return program_new_action_with_direction (HUNT, dir);
        case BREEDT: return program_new_action_with_direction (BREED, dir);
        case RESTT:  return program_new_action_with_time (time);
        case MOVET:  return program_new_action_with_direction (MOVE, dir);
    }
}

/*******************************************************************************
* The lexer has returned an identifier token at the start of the statement. This
* could only mean an assign statement.
*
* vars      The list of progam variables to potentially assign to.
* return    The assign statement. NULL if error.
*******************************************************************************/
Statement *
compiler_get_assign (GHashTable *vars)
{
    Identifier *identifier;
    Expression *expression;
    Token token;
    const char* name;

    name = lexer_get_string_value ();
    Identifier *var = g_hash_table_lookup (vars, name);
    
    token = lexer_get_token ();
    if (token != ONEEQT)
    {
        compiler_error ("missing '=' in assign statement\n");
        return NULL;
    }

    expression = compiler_get_expression (vars);
    if (expression == NULL)
        return NULL;

    return program_new_assign (var, expression);
}

/*******************************************************************************
* Returns the next if then statement from the lexer stream. The if token has
* already been discovered.
*
* vars          The table of program variables.
* return        A statement initialised as a if then statement. NULL if error.
*******************************************************************************/
static Statement *
compiler_get_if_then (GHashTable *vars)
{
    Condition *condition;
    Statement *true_statement;
    Statement *else_statement;
    Token token;    

    token = lexer_get_token ();
    condition = compiler_get_condition (token, vars);
    if (condition == NULL)
        return NULL;

    token = lexer_get_token ();
    if (token != THENT)
    {
        program_free_condition (condition);
        return NULL;
    }

    token = lexer_get_token ();
    true_statement = compiler_get_statement (token, vars);
    if (true_statement == NULL)
    {
        program_free_condition (condition);
        return NULL;
    }

    token = lexer_get_token ();
    if (token == ELSET)
    {
        token = lexer_get_token ();
        else_statement = compiler_get_statement (token, vars);
        if (else_statement == NULL)
        {
            program_free_condition (condition);
            program_free_statement (true_statement);
            return NULL;
        }
        token = lexer_get_token ();
    }
    else 
        else_statement = NULL;

    if (token != ENDT)
    {
        program_free_condition (condition);
        program_free_statement (true_statement);
        program_free_statement (else_statement);
        compiler_error ("end keyword expected\n");
        return NULL;
    }

    return program_new_if_then (condition, true_statement, else_statement);
}

/*******************************************************************************
* Creates a block statement. The block token has already been discovered.
* The form of the block statement is: "block" { statement } "end".
*
* vars      The table containing the program variables.
* return    A new block statement. NULL if error.
*******************************************************************************/
static Statement *
compiler_get_block (GHashTable *vars)
{
    GList *list;
    Statement *statement;
    Token token;

    list = NULL;
    token = lexer_get_token ();  
  
    while (token != ENDT || token != EOFT)
    {
        statement = compiler_get_statement (token, vars);
        if (statement == NULL)
        {
            g_list_free_full (list, program_free_statement_void);
            return NULL;
        }
        list = g_list_append (list, statement);
        token = lexer_get_token ();
    }

    if (token == EOFT)
    {
        g_list_free_full (list, program_free_statement_void);
        compiler_error ("end of block not found\n");
        return NULL;
    }

    return program_new_block (list);
}

/*******************************************************************************
* Analyses a token that is supposed to start a statement and calls the correct
* constructor. If the token is not correct for the start of the statement NULL
* is returned.
*
* token         The first token must already have been discoved.
* vars          The table containing the program variables.
* return        The next statment in the lexer stream.
*******************************************************************************/
static Statement *
compiler_get_statement (Token token, GHashTable *vars)
{
    switch (token)
    {
        case BLOCKT:    return compiler_get_block (vars);
        case IFT:       return compiler_get_if_then (vars);
        case IDT:       return compiler_get_assign (vars);
        case MOVET:
        case HUNTT:
        case BREEDT:
        case RESTT:     return compiler_get_action (token);
    }
    compiler_error ("improper start of statement\n");
    return NULL;
}

/*******************************************************************************
* Creates a new program. This involves checking for the 'sim' keyword, the 
* statement of the program, and the 'end' keyword.
*
* file      The file containing the sim2 source.
* return    The new program. If there is an error it will be null.
*******************************************************************************/
Program *
compiler_get_program (GFile *file)
{
    Token token;
    Statement *statement;
    GHashTable *vars;

    lexer_load_file (file);
    vars = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
    compiler_find_variables (vars);

    lexer_load_file (file);
    token = lexer_get_token ();    
    if (token != SIMT)
    {
        g_hash_table_destroy (vars);
        compiler_error ("no sim keyword\n");
        return NULL;
    }

    token = lexer_get_token ();
    statement = compiler_get_statement (token, vars);
    if (statement == NULL)
    {
        g_hash_table_destroy (vars);
        return NULL;
    }

    token = lexer_get_token ();
    if (token != ENDT)
    {
        program_free_statement (statement);
        g_hash_table_destroy (vars);
        compiler_error ("missing end keyword\n");
        return NULL;
    }

    return program_new (statement, vars);
}

