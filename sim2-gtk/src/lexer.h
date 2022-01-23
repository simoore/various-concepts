#ifndef _LEXER_H_
#define _LEXER_H_

typedef enum {EOFT, NULLT, IDT, INTT, SIMT, LPART, RPART, ENDT, IFT, BLOCKT,
    THENT, ELSET, RANDT, ONEEQT, ADDT, SUBT, LTT, GTT, TWOEQT, MOVET, HUNTT,
    RESTT, BREEDT, IDRESTT, IDENERGYT, NT, NET, ET, SET, ST, SWT, WT, NWT} 
    Token;

void
lexer_load_file (GFile *file);

const char *
lexer_get_string_value ();

int
lexer_get_int_value ();

int
lexer_get_line ();

Token 
lexer_get_token ();

Token
lexer_uninit ();

#endif /* _LEXER_H_ */

