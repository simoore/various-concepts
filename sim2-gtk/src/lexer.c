#include <stdlib.h>
#include <gio/gio.h>
#include "lexer.h"
#include "program.h"

#define INITIAL_STATE    0
#define IDENTIFIER_STATE 1
#define OPERATOR_STATE   2
#define NUMBER_STATE     3
#define ERROR_STATE      4
#define STOP_STATE       5

#define WHITE   0
#define DIGIT   1
#define LETTER  2
#define PUNC    3
#define NO_TYPE 4

static int column = 0;
static gsize length = 0;
static char *line = NULL;
static char *token = NULL;
static GDataInputStream *stream = NULL;
static GFileInputStream *file_input_stream = NULL;
static int transition[4][5];
static int line_number;

static void lexer_init ();
static Token lexer_process_identifier (char *token);
static Token lexer_process_operator (char *token);

/*******************************************************************************
* Loads a new file into the lexer for analysis. Assumes that any previous file
* has been purged with a call to lexer_uninit ().
*
* file      The reference to the file that is to be tokenized.
*******************************************************************************/
void
lexer_load_file (GFile *file)
{
    line_number = 0;
    file_input_stream = g_file_read (file, NULL, NULL);
    if (file_input_stream != NULL)
        stream = g_data_input_stream_new (G_INPUT_STREAM (file_input_stream));
    lexer_init ();
}

/*******************************************************************************
* Returns the string value of the token so the parser can keep a list of 
* identifiers that may appear in many parts of the code.
*
* return    The string representing the token.
*******************************************************************************/
const char *
lexer_get_string_value ()
{
    return token;
}

/*******************************************************************************
* Returns the int value of the token for the creation of constant integer.
* Only call this function if you are sure the token is an integer as no checks
* are performed.
*
* return    The integer value of the token.
*******************************************************************************/
int
lexer_get_int_value ()
{
    return atoi (token);
}

/*******************************************************************************
* Returns the current line number that the lexer is analysing.
*
* return    The line number.
*******************************************************************************/
int
lexer_get_line ()
{
    return line_number;
}

/*******************************************************************************
* This function reads the textfile and indentifies the next token. The state of
* the last token identification process is remembered and the lexer starts where
* it left off. If it desired to go backwards, then the lexer should be reloaded
* and started again.
*
* return        The next token in the lexer's input stream.
*******************************************************************************/
Token 
lexer_get_token ()
{
    GError *err = NULL;
    int final_state = INITIAL_STATE;
    int state = INITIAL_STATE;
    int type = NO_TYPE;
    int start_column = column;
    char character;
    char punc_string[] = {' ', '\0'};
    
    if (stream == NULL) return NULLT;
    
    // analyse text file stream to retrieve token
    while ((state != ERROR_STATE) && (state != STOP_STATE))
    {   
        // execute if the token starts on the new line
        if (column == length && state == INITIAL_STATE)
        {
            line_number++;
            column = 0;
            g_free (line);
            line = g_data_input_stream_read_line (stream, &length, NULL, &err);
            if (err != NULL)    
            {
                g_error_free (err);   
                return lexer_uninit ();
            }
        }
        
        // if not at end of file
        if (line != NULL)
        {
            // determine the character
            if (column != length) character = line[column];
            else                  character = '\n';

            // move column indexes
            if (state == INITIAL_STATE) start_column = column;
            if (column != length)       column++;

            // determine character type and perform lexer state transition
            punc_string[0] = character;
            if      (g_ascii_isspace (character) == TRUE)       type = WHITE;
            else if (g_ascii_isalpha (character) == TRUE)       type = LETTER;
            else if (g_ascii_isdigit (character) == TRUE)       type = DIGIT;
            else if (g_strrstr ("()<>=", punc_string) != NULL)  type = PUNC;
            else                                                type = NO_TYPE;
            final_state = state;
            state = transition[state][type];                    
        }
        else
        {
            final_state = INITIAL_STATE;
            state = STOP_STATE;
        }
    }

    // process a token error
    if (state == ERROR_STATE)   return lexer_uninit ();

    // process the identified token
    if (final_state != INITIAL_STATE)
    {
        if (column != length) column--;
        g_free (token);
        token = g_strndup (line + start_column, column - start_column);
        g_strstrip (token);
    }
    else    lexer_uninit ();
    switch (final_state)
    {
        case INITIAL_STATE:     return EOFT; break;
        case IDENTIFIER_STATE:  return lexer_process_identifier (token); break;
        case OPERATOR_STATE:    return lexer_process_operator (token); break;
        case NUMBER_STATE:      return INTT; break;
    }   
}

/*******************************************************************************
* Initialises the transistions table used in the lexing process.
*******************************************************************************/
static void
lexer_init ()
{
    static gboolean initialised = FALSE;
    
    if (initialised == TRUE)    return;
        
    transition[INITIAL_STATE][WHITE] = INITIAL_STATE;
    transition[INITIAL_STATE][DIGIT] = NUMBER_STATE;
    transition[INITIAL_STATE][LETTER] = IDENTIFIER_STATE;
    transition[INITIAL_STATE][PUNC] = OPERATOR_STATE;
    transition[INITIAL_STATE][NO_TYPE] = ERROR_STATE;

    transition[IDENTIFIER_STATE][WHITE] = STOP_STATE;
    transition[IDENTIFIER_STATE][LETTER] = IDENTIFIER_STATE;
    transition[IDENTIFIER_STATE][DIGIT] = IDENTIFIER_STATE;
    transition[IDENTIFIER_STATE][PUNC] = STOP_STATE;
    transition[IDENTIFIER_STATE][NO_TYPE] = ERROR_STATE;

    transition[OPERATOR_STATE][WHITE] = STOP_STATE;
    transition[OPERATOR_STATE][LETTER] = STOP_STATE;
    transition[OPERATOR_STATE][DIGIT] = STOP_STATE;
    transition[OPERATOR_STATE][PUNC] = OPERATOR_STATE;
    transition[OPERATOR_STATE][NO_TYPE] = ERROR_STATE;

    transition[NUMBER_STATE][WHITE] = STOP_STATE;
    transition[NUMBER_STATE][LETTER] = ERROR_STATE;
    transition[NUMBER_STATE][DIGIT] = NUMBER_STATE;
    transition[NUMBER_STATE][PUNC] = STOP_STATE;
    transition[NUMBER_STATE][NO_TYPE] = ERROR_STATE;
    
    initialised = TRUE;
}

/*******************************************************************************
* Uninitialises the lexer after an error or the end of file is found.
*
* err       A g_error associated with IO that if not NULL will be freed.
* return    A null token associated with an uninitialised lexer that can be 
*           returned to the parser.
*******************************************************************************/
Token
lexer_uninit ()
{
    g_free (line);
    g_free (token);
    column = 0;
    length = 0;
    line = NULL;
    token = NULL;
    if (stream != NULL) g_object_unref (file_input_stream);
    stream = NULL;
    file_input_stream = NULL; 
    return NULLT;
}

/*******************************************************************************
* Identifies the keywords from identifiers.
*
* token     The text string representing the token.
* return    The token that the string represents.
*******************************************************************************/
static Token
lexer_process_identifier (char *token)
{
    if (g_strcmp0 ("sim", token) == 0)          return SIMT;
    if (g_strcmp0 ("if", token) == 0)           return IFT;
    if (g_strcmp0 ("end", token) == 0)          return ENDT;
    if (g_strcmp0 ("then", token) == 0)         return THENT;
    if (g_strcmp0 ("else", token) == 0)         return ELSET;
    if (g_strcmp0 ("move", token) == 0)         return MOVET;
    if (g_strcmp0 ("block", token) == 0)        return BLOCKT;
    if (g_strcmp0 ("rest", token) == 0)         return RESTT;
    if (g_strcmp0 ("breed", token) == 0)        return BREEDT;
    if (g_strcmp0 ("hunt", token) == 0)         return HUNTT;
    if (g_strcmp0 ("add", token) == 0)          return ADDT;
    if (g_strcmp0 ("rand", token) == 0)         return RANDT;
    if (g_strcmp0 ("awakeDaily", token) == 0)   return IDRESTT;
    if (g_strcmp0 ("energy", token) == 0)       return IDENERGYT;
    if (g_strcmp0 ("N", token) == 0)            return NT;
    if (g_strcmp0 ("NE", token) == 0)           return NET;
    if (g_strcmp0 ("E", token) == 0)            return ET;
    if (g_strcmp0 ("SE", token) == 0)           return SET;
    if (g_strcmp0 ("S", token) == 0)            return ST;
    if (g_strcmp0 ("SW", token) == 0)           return SWT;
    if (g_strcmp0 ("W", token) == 0)            return WT;
    if (g_strcmp0 ("NW", token) == 0)           return NWT;
    return IDT;
}

/*******************************************************************************
* Identifies the type of operator.
*
* token     The string value of the token.
* return    The token the string represents.
*******************************************************************************/
static Token
lexer_process_operator (char *token)
{
    if (g_strcmp0 ("(", token) == 0)    return LPART;
    if (g_strcmp0 (")", token) == 0)    return RPART;
    if (g_strcmp0 ("==", token) == 0)   return TWOEQT;
    if (g_strcmp0 ("=", token) == 0)    return ONEEQT;
    if (g_strcmp0 ("<", token) == 0)    return LTT;
    if (g_strcmp0 (">", token) == 0)    return GTT;
    return NULLT;
}

