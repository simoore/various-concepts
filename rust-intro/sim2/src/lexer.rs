use std::fmt;

/*****************************************************************************/
/********** TYPES ************************************************************/
/*****************************************************************************/

#[derive(Clone, Debug)]
pub enum Token {
    Id(String), 
    Int(i32), 
    Sim, 
    Lpar, 
    Rpar, 
    End, 
    If, 
    Block,
    Then, 
    Else, 
    Rand, 
    OneEq, 
    Add, 
    Sub, 
    Lt, 
    Gt, 
    TwoEq, 
    Move, 
    Hunt,
    Rest, 
    Breed, 
    IdRest, 
    IdEnergy, 
    DirN, 
    DirNE, 
    DirE, 
    DirSE, 
    DirS, 
    DirSW, 
    DirW, 
    DirNW,
}

#[derive(PartialEq)]
enum State {
    Initial,
    Identifier,
    Operator,
    Number,
    Stop
}

#[derive(PartialEq)]
enum CharType {
    White,
    Digit,
    Letter,
    Punc,
}

#[derive(Debug)]
pub enum LexerError {
    InvalidCharType,
    InvalidTransition,
    InvalidOperatorString,
    InvalidTokenState,
    ParseNumberError,
}

/*****************************************************************************/
/********** FUNCTIONS ********************************************************/
/*****************************************************************************/

impl From<std::num::ParseIntError> for LexerError {
    fn from(_: std::num::ParseIntError) -> Self {
        Self::ParseNumberError
    }
}


/// Implements equality for the Token enum.
impl PartialEq for Token {
    fn eq(&self, other: &Self) -> bool {
        if std::mem::discriminant(self) == std::mem::discriminant(other) {
            match (self, other) {
                (Token::Int(a), Token::Int(b)) => a == b,
                (Token::Id(a), Token::Id(b)) => a == b,
                _ => true,
            }
        } else {
            false
        }
    }
}


/// This is required to implement the std::error::Error trait for LexerError.
impl std::error::Error for LexerError {}


/// This allows LexerErrors to be used with a println! formatter.
impl fmt::Display for LexerError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        let s = match self {
            LexerError::InvalidCharType => "InvalidCharType",
            LexerError::InvalidTransition => "InvalidTransition",
            LexerError::InvalidOperatorString => "InvalidOperatorString",
            LexerError::InvalidTokenState => "InvalidTokenState",
            LexerError::ParseNumberError => "ParseNumberError",
        };
        write!(f, "{}", &s)
    }
}


/// Converts the sim2 code into a vector of tokens.
/// 
/// contents
///      The sim2 string.
/// returns
///      The vector of tokens if there are not parsing errors.
pub fn tokenize(contents: &str) -> Result<Vec<Token>, LexerError> {
    let mut head = 0;
    let mut tail = 0;
    let mut state = State::Initial;
    let mut tokens: Vec<Token> = vec![];
    for c in contents.chars() {
        let ct = char_type(c)?;
        let mut new_state = transition(&ct, &state)?;

        match new_state {
            State::Initial => { tail = head; }
            State::Stop => {
                let slice = &contents[tail..head];
                let token = str2token(&slice, &state)?;
                tokens.push(token);
                tail = head;
                // process current character again
                new_state = transition(&ct, &State::Initial)?;
            }
            _ if state == State::Initial => { tail = head; }
            _ => {}
        }
        state = new_state;
        head += 1;
    }
    Ok(tokens)
}

/// Coverts the framed token string to a token.
/// 
/// slice
///     The string representation of the token.
/// state
///     The state of the lexer when the token was discovered.
fn str2token(slice: &str, state: &State) -> Result<Token, LexerError> {
    match state {
        State::Identifier => Ok(process_identifier(slice)),
        State::Operator => Ok(process_operator(slice)?),
        State::Number => slice.parse::<i32>().map(|x| Token::Int(x)).map_err(|_| LexerError::ParseNumberError),
        _ => Err(LexerError::InvalidTokenState),
    }
}

/// Labels the type of char that the lexer is processing.
/// 
/// c
///     The char to label.
/// returns
///     The type of char, or error if its not support by this lexer.
fn char_type(c: char) -> Result<CharType, LexerError> {
    if c.is_ascii_whitespace() {
        Ok(CharType::White)
    } else if c.is_ascii_alphabetic() {
        Ok(CharType::Letter)
    } else if c.is_ascii_digit() {
        Ok(CharType::Digit)
    } else if "()<>=".contains(c) {
        Ok(CharType::Punc)
    } else {
        Err(LexerError::InvalidCharType)
    }
}

/// Examines the current state of the lexer amd the current character it is processing and adjusts the state.
/// This is used to frame valid tokens in the stream.
/// 
/// c 
///     The current character the lexer is processing.
/// s
///     The current state of the lexer.
/// return
///     The new state of the lexer.
fn transition(ct: &CharType, s: &State) -> Result<State, LexerError> {
    match s {
        State::Initial => match ct {
            CharType::White => Ok(State::Initial),
            CharType::Digit => Ok(State::Number),
            CharType::Letter => Ok(State::Identifier),
            CharType::Punc => Ok(State::Operator),
        },
        State::Identifier => match ct {
            CharType::White => Ok(State::Stop),
            CharType::Digit => Ok(State::Identifier),
            CharType::Letter => Ok(State::Identifier),
            CharType::Punc => Ok(State::Stop),
        },
        State::Operator => match ct {
            CharType::White => Ok(State::Stop),
            CharType::Digit => Ok(State::Stop),
            CharType::Letter => Ok(State::Stop),
            CharType::Punc => Ok(State::Operator),
        },
        State::Number => match ct {
            CharType::White => Ok(State::Stop),
            CharType::Digit => Ok(State::Number),
            CharType::Letter => Err(LexerError::InvalidTransition),
            CharType::Punc => Ok(State::Stop),
        },
        State::Stop => match ct {
            CharType::White => Ok(State::Initial),
            CharType::Digit => Ok(State::Number),
            CharType::Letter => Ok(State::Identifier),
            CharType::Punc => Ok(State::Operator),
        },
    }
} 

/// Identifies the keywords from identifiers.
///
/// token     
///     The text string representing the token.
/// return
///     The token that the string represents.
fn process_identifier(identifier_str: &str) -> Token {
    match identifier_str {
        "sim" => Token::Sim,
        "if" => Token::If,
        "end" => Token::End,
        "then" => Token::Then,
        "else" => Token::Else,
        "move" => Token::Move,
        "block" => Token::Block,
        "rest" => Token::Rest,
        "breed" => Token::Breed,
        "hunt" => Token::Hunt,
        "add" => Token::Add,
        "sub" => Token::Sub,
        "rand" => Token::Rand,
        "awakeDaily" => Token::IdRest,
        "energy" => Token::IdEnergy,
        "N" => Token::DirN,
        "NE" => Token::DirNE,
        "E" => Token::DirE,
        "SE" => Token::DirSE,
        "S" => Token::DirS,
        "SW" => Token::DirSW,
        "W" => Token::DirW,
        "NW" => Token::DirNW,
        _ => Token::Id(String::from(identifier_str)),
    }
}

/// Identifies the type of operator.
///
/// operator_str     
///     The string value of the token.
/// return    
///     The token the string represents.
fn process_operator(operator_str: &str) -> Result<Token, LexerError> {
    match operator_str {
        "(" => Ok(Token::Lpar),
        ")" => Ok(Token::Rpar),
        "=" => Ok(Token::OneEq),
        "==" => Ok(Token::TwoEq),
        "<" => Ok(Token::Lt),
        ">" => Ok(Token::Gt),
        _ => Err(LexerError::InvalidOperatorString),
    }
}

/*****************************************************************************/
/********** TESTS ************************************************************/
/*****************************************************************************/

#[cfg(test)]
mod tests {
    use super::*;
    use crate::code::CODE;

static EXPECTED_TOKENS: [Token; 25]  = [Token::Sim, Token::If, Token::Lpar, Token::IdEnergy, Token::Gt, 
    Token::Int(0), Token::Rpar, Token::Then, Token::If, Token::Lpar, Token::IdRest, Token::Gt, Token::Int(16), 
    Token::Rpar, Token::Then, Token::Rest, Token::Int(8), Token::Else, Token::If, Token::Lpar, Token::IdEnergy, 
    Token::Gt, Token::Int(100), Token::Rpar, Token::Then];

    #[test]
    fn test_tokenize() {
        let tokens = tokenize(CODE).unwrap();
        for (t, et) in std::iter::zip(&tokens, &EXPECTED_TOKENS) {
            assert_eq!(et, t);
        }
    }
}