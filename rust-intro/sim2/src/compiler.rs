use crate::lexer::{LexerError, Token, tokenize};
use crate::program::{Action, Arithmetic, ArithmeticOp, Assign, Block, Condition, ConditionOp, Direction, 
    Expression, Identifier, IfThen, Program, SpecialIdentifier, Statement, Variable};

/*****************************************************************************/
/********** COMPILER ERROR ENUMS *********************************************/
/*****************************************************************************/

#[derive(Clone, Debug)]
pub enum CompilerError {
    BreedNotFollowedByDirection,
    ConditionMissingLPar,
    ConditionMissingOperator,
    ConditionMissingRPar,
    EndOfProgramNotLastToken,
    HuntNotFollowedByDirection,
    ImproperStartOfStatment,
    InvalidAssignStatement,
    InvalidIdentifier,
    MoveNotFollowedByDirection,
    NoAddOrSubstract,
    NoConditionOperator,
    NoEndBlockFound,
    NoEndKeyword,
    NoElseOrEnd,
    NoExpressionFound,
    NoThenKeyword,
    NotADirectionToken,
    NoSimKeyword,
    RestNotFollowedByCount,
}

#[derive(Debug)]
pub enum CompilerOrLexerError {
    C(CompilerError),
    L(LexerError),
}

/*****************************************************************************/
/********** RETURN TYPES *****************************************************/
/*****************************************************************************/

type ArithmeticReturn<'a> = (Arithmetic, &'a[Token]);
type ExpressionReturn<'a> = (Box<dyn Expression>, &'a[Token]);
type ConditionReturn<'a> = (Condition, &'a[Token]);
type IdentifierReturn<'a> = (Identifier, &'a[Token]);
type StatementReturn<'a> = (Box<dyn Statement>, &'a[Token]);

/*****************************************************************************/
/********** MAPPING FUNCTIONS ************************************************/
/*****************************************************************************/

fn map_direction_token(x: &Token) -> Result<Direction, CompilerError> {
    match x {
        Token::DirN => Ok(Direction::N),
        Token::DirNE => Ok(Direction::NE), 
        Token::DirE => Ok(Direction::E), 
        Token::DirSE => Ok(Direction::SE), 
        Token::DirS => Ok(Direction::S), 
        Token::DirSW => Ok(Direction::SW), 
        Token::DirW => Ok(Direction::W), 
        Token::DirNW => Ok(Direction::NW),
        _ => Err(CompilerError::NotADirectionToken),
    }
}

fn map_operator_token(x: &Token) -> Result<ConditionOp, CompilerError> {
    match x {
        Token::Lt => Ok(ConditionOp::Lt),
        Token::Gt => Ok(ConditionOp::Gt), 
        Token::TwoEq => Ok(ConditionOp::Eq), 
        _ => Err(CompilerError::NoConditionOperator),
    }
}

fn map_arithmetic_operator_token(x: &Token) -> Result<ArithmeticOp, CompilerError> {
    match x {
        Token::Add => Ok(ArithmeticOp::Add),
        Token::Sub => Ok(ArithmeticOp::Sub), 
        _ => Err(CompilerError::NoAddOrSubstract),
    }
}

/*****************************************************************************/
/********** HELPER FUNCTIONS *************************************************/
/*****************************************************************************/

fn head_token(tokens: &[Token], error: CompilerError) -> Result<(&Token, &[Token]), CompilerError> {
    let token = tokens.get(0).ok_or(error.clone())?;
    Ok((token, &tokens[1..]))
}

fn consume_token(token: Token, tokens: &[Token], error: CompilerError) -> Result<&[Token], CompilerError> {
    let (first_token, tokens) = head_token(tokens, error.clone())?;
    if *first_token != token { Err(error) } else { Ok(tokens) }
}

/*****************************************************************************/
/********** GET EXPRESSIONS **************************************************/
/*****************************************************************************/


/// Finds an identifier at the start of the token stream.
///
/// @param tokens
///     The unprocessed token stream.
/// @return
///     The identifier if found, otherwise a compiler error.
fn get_identifier(tokens: &[Token]) -> Result<IdentifierReturn, CompilerError> {
    let (token, tokens) = head_token(tokens, CompilerError::InvalidIdentifier)?;
    match token {
        Token::Id(key) => Ok((Identifier::new_variable(key.to_string()), tokens)),
        Token::Int(val) => Ok((Identifier::new_constant(*val), tokens)),
        Token::IdRest => Ok((Identifier::new_specialized(SpecialIdentifier::DailyRest), tokens)),
        Token::IdEnergy => Ok((Identifier::new_specialized(SpecialIdentifier::Energy), tokens)),
        Token::Rand => Ok((Identifier::new_specialized(SpecialIdentifier::Rand), tokens)),
        _ => Err(CompilerError::InvalidIdentifier),
    }
}


/// Finds a condition expression.
///
/// @param tokens
///     The tokens of the sim2 program. A valid condition expression is expected at the start of this list.
/// @return
///     The condition expression found + the tokens minus the ones consumed for the condition expression. A
///     compiler error is returned if something went wrong.
fn get_condition(tokens: &[Token]) -> Result<ConditionReturn, CompilerError> {
    let tokens = consume_token(Token::Lpar, tokens, CompilerError::ConditionMissingLPar)?;
    let (identifier_a, tokens) = get_identifier(tokens)?;
    let (token, tokens) = head_token(tokens, CompilerError::ConditionMissingOperator)?;
    let op = map_operator_token(&token)?;
    let (identifier_b, tokens) = get_identifier(tokens)?;
    let tokens = consume_token(Token::Rpar, tokens, CompilerError::ConditionMissingRPar)?;
    Ok((Condition::new(identifier_a, identifier_b, op), tokens))
}


/// Finds an arithmetic expression at the start of the unprocessed token stream.
///
/// @param tokens
///     The unprocessed token stream.
/// @return
///     The arithmetic expression with the remaining unprocessed tokens or a compiler error.
fn get_arithmetic(tokens: &[Token]) -> Result<ArithmeticReturn, CompilerError> {
    let (token, tokens) = head_token(tokens, CompilerError::NoAddOrSubstract)?;
    let op = map_arithmetic_operator_token(token)?;
    let (identifier_a, tokens) = get_identifier(tokens)?;
    let (identifier_b, tokens) = get_identifier(tokens)?;
    Ok((Arithmetic::new(identifier_a, identifier_b, op), tokens))
}


/// Finds an expression at the start of the unprocessed token stream. It checks the first token, but doesn't consume
/// it.
///
/// @param tokens
///     The stream of unprocessed tokens.
fn get_expression(tokens: &[Token])  -> Result<ExpressionReturn, CompilerError> {
    let token = tokens.get(0).ok_or(CompilerError::NoExpressionFound)?;
    match *token {
        Token::Lpar => get_condition(tokens)
            .map(|(cond, tkns)| (Box::new(cond) as Box<dyn Expression>, tkns)),
        Token::Add | Token::Sub => get_arithmetic(tokens)
            .map(|(arith, tkns)| (Box::new(arith) as Box<dyn Expression>, tkns)),
        _ => get_identifier(tokens)
            .map(|(id, tkns)| (Box::new(id) as Box<dyn Expression>, tkns)),
    }
}

/*****************************************************************************/
/********** GET STATEMENTS ***************************************************/
/*****************************************************************************/

/// We have consumed the move keyword and now are checking for the direction of the move statement.
fn get_move(tokens: &[Token]) -> Result<StatementReturn, CompilerError> {
    let (token, tokens) = head_token(tokens, CompilerError::MoveNotFollowedByDirection)?;
    let dir = map_direction_token(token)?;
    Ok((Box::new(Action::Move(dir)), tokens))
}

fn get_hunt(tokens: &[Token]) -> Result<StatementReturn, CompilerError> {
    let (token, tokens) = head_token(tokens, CompilerError::HuntNotFollowedByDirection)?;
    let dir = map_direction_token(token)?;
    Ok((Box::new(Action::Hunt(dir)), tokens))
}

fn get_breed(tokens: &[Token]) -> Result<StatementReturn, CompilerError> {
    let (token, tokens) = head_token(tokens, CompilerError::BreedNotFollowedByDirection)?;
    let dir = map_direction_token(token)?;
    Ok((Box::new(Action::Breed(dir)), tokens))
}

fn get_rest(tokens: &[Token]) -> Result<StatementReturn, CompilerError> {
    let (token, tokens) = head_token(tokens, CompilerError::RestNotFollowedByCount)?;
    if let Token::Int(count) = token {
        Ok((Box::new(Action::Rest(*count)), tokens))
    } else {
        Err(CompilerError::RestNotFollowedByCount)
    }
}

/// Finds an assign statement at the start of the unprocessed token stream.
///
/// @param tokens
///     The unprocessed token stream.
/// @param key
///     The variable name we are assigning to.
/// @return
///     The new assign statement or a compiler error is something went wrong.
fn get_assign<'a>(tokens: &'a[Token], key: &String) -> Result<StatementReturn<'a>, CompilerError> {
    let var = Variable::new(key.clone());
    let tokens = consume_token(Token::OneEq, tokens, CompilerError::InvalidAssignStatement)?;
    let (expr, tokens) = get_expression(tokens)?;
    Ok((Box::new(Assign::new(var, expr)) , tokens))
}


/// An if token has been detect so we now analyze the set or proceeding tokens to constuct the ifthen statement.
/// The if token is already consumed.
fn get_ifthen(tokens: &[Token]) -> Result<StatementReturn, CompilerError> {
    let (condition, tokens) = get_condition(tokens)?;
    let tokens = consume_token(Token::Then, tokens, CompilerError::NoThenKeyword)?;
    let (then_statement, tokens) = get_statement(tokens)?;
    let (else_or_end, tokens) = head_token(tokens, CompilerError::NoElseOrEnd)?;
    if *else_or_end == Token::End {
        return Ok((Box::new(IfThen::new(condition, then_statement, None)), tokens));
    } else if *else_or_end != Token::Else {
        return Err(CompilerError::NoElseOrEnd);
    }
    let (else_statement, tokens) = get_statement(tokens)?;
    let tokens = consume_token(Token::End, tokens, CompilerError::NoEndKeyword)?;
    Ok((Box::new(IfThen::new(condition, then_statement, Some(else_statement))), tokens))
}


fn get_block(tokens: &[Token])  -> Result<StatementReturn, CompilerError> {
    let mut statements = Vec::new();
    let mut loop_tokens = tokens; 
    loop {
        let token = loop_tokens.get(0).ok_or(CompilerError::NoEndBlockFound)?;
        if *token == Token::End {
            return Ok((Box::new(Block::new(statements)), &loop_tokens[1..]));
        }
        let (statement, tkns) = get_statement(loop_tokens)?;
        loop_tokens = tkns;
        statements.push(statement);
    }
}


/// Analyses a token that is supposed to start a statement and calls the correct constructor. If the token is not 
/// correct for the start of the statement and error is returned.
///
/// @param tokens     
///     The unconsummed token stream.
/// @param vars          
///     The table containing the program variables.
/// @return        
///     The next statment in the lexer stream.
fn get_statement(tokens: &[Token]) -> Result<StatementReturn, CompilerError> {
    let (token, tokens) = head_token(tokens, CompilerError::ImproperStartOfStatment)?;
    match token {
        Token::Block => get_block(tokens),
        Token::If => get_ifthen(tokens),
        Token::Id(key) => get_assign(tokens, key),
        Token::Move => get_move(tokens),
        Token::Hunt => get_hunt(tokens),
        Token::Breed => get_breed(tokens),
        Token::Rest => get_rest(tokens),
        _ => Err(CompilerError::ImproperStartOfStatment)
    }
}


/// Creates a new program. This involves checking for the 'sim' keyword, the statement of the program, and the 
/// 'end' keyword.
///
/// @param text      
///     The sim2 program.
/// @return
///     The new program.
pub fn get_program(text: &str) -> Result<Program, CompilerOrLexerError> {
    let tokens = tokenize(text)
        .map_err(|e| CompilerOrLexerError::L(e))?;
    let tokens = consume_token(Token::Sim, &tokens, CompilerError::NoSimKeyword)
        .map_err(|e| CompilerOrLexerError::C(e))?;
    let (statement, tokens) = get_statement(tokens)
        .map_err(|e| CompilerOrLexerError::C(e))?;
    consume_token(Token::End, tokens, CompilerError::EndOfProgramNotLastToken)
        .map_err(|e| CompilerOrLexerError::C(e))?;
    // TODO: we don't handle EOF well and we miss the last END token if a newline or whitespace is not there.
    Ok(Program::new(statement))
}

/*****************************************************************************/
/********** TESTS ************************************************************/
/*****************************************************************************/

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_consume_token() {
        let tokens = vec![Token::DirE];
        let tokens = consume_token(Token::DirE, &tokens, CompilerError::NotADirectionToken).unwrap();
        assert_eq!(tokens.len(), 0);
    }

    #[test]
    fn test_move() {
        let tokens = vec![Token::DirE];
        let (statement, _) = get_move(&tokens[..]).unwrap();
        assert_eq!(statement.to_string(), "Move(E)");
    }

    #[test]
    fn test_hunt() {
        let tokens = vec![Token::DirNW];
        let (statement, _) = get_hunt(&tokens[..]).unwrap();
        assert_eq!(statement.to_string(), "Hunt(NW)");
    }

    #[test]
    fn test_breed() {
        let tokens = vec![Token::DirNE];
        let (statement, _) = get_breed(&tokens[..]).unwrap();
        assert_eq!(statement.to_string(), "Breed(NE)");
    }

    #[test]
    fn test_rest() {
        let tokens = vec![Token::Int(7)];
        let (statement, _) = get_rest(&tokens[..]).unwrap();
        assert_eq!(statement.to_string(), "Rest(7)");
    }

    #[test]
    fn test_identifier_rand() {
        let tokens = vec![Token::Rand];
        let (id, _) = get_identifier(&tokens).unwrap();
        assert_eq!(id.to_string(), "Rand");
    }

    #[test]
    fn test_identifier_constant() {
        let tokens = vec![Token::Int(7)];
        let (id, _) = get_identifier(&tokens).unwrap();
        assert_eq!(id.to_string(), "7");
    }

    #[test]
    fn test_condition() {
        let tokens = vec![Token::Lpar, Token::Int(7), Token::Lt, Token::Int(30), Token::Rpar];
        let (cond, _) = get_condition(&tokens).unwrap();
        assert_eq!(cond.to_string(), "( 7 < 30 )");
    }

    #[test]
    fn test_ifthen() {
        let tokens = vec![Token::If, Token::Lpar, Token::Int(7), Token::Lt, Token::Int(30), Token::Rpar, 
            Token::Then, Token::Move, Token::DirNE, Token::End];
        let (statement, _) = get_ifthen(&tokens[1..]).unwrap();
        assert_eq!(statement.to_string(), "if ( 7 < 30 ) then Move(NE) end");
    }

    #[test]
    fn test_assign() {
        let tokens = vec![Token::OneEq, Token::Int(7)];
        let key = "myvar".to_string();
        let (statement, _) = get_assign(&tokens, &key).unwrap();
        assert_eq!(statement.to_string(), "myvar = 7");
    }

    #[test]
    fn test_block() {
        let tokens = vec![Token::Move, Token::DirE, Token::Hunt, Token::DirSW, Token::End];
        let (statement, _) = get_block(&tokens).unwrap();
        assert_eq!(statement.to_string(), "Block Move(E) Hunt(SW) End");
    }
}