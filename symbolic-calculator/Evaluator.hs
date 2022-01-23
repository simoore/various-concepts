module Evaluator (SymTab, evaluate, runEvaluator, initSymTab) where

import Lexer
import Parser
import Data.Char
import qualified Data.Map as M
import Control.Monad.State

type SymTab = M.Map String Double

type Evaluator a = State SymTab a

lookUp :: String -> Evaluator Double
lookUp str = do
    symTab <- get
    case M.lookup str symTab of
        Just v -> return v
        Nothing -> error $ "undefined variable " ++ str

-- Since its return type is the state monad type Evaluator, this function can
-- be bound to Evaluator types. This allows the state of the evaluator to 
-- be passed to this function. This is what allows the function to retrieve
-- the state of the evaluator and set a new state with the get and put 
-- functions.
addSymbol :: String -> Double -> Evaluator ()
addSymbol str val = do
    symTab <- get
    put $ M.insert str val symTab
    return ()

-- A tree produced by the parser is passed to the evaluator to have is value
-- calculated. The reason it is wrapped in an Evaluator type is to allow
-- a state to be passed to each execution of the evaluator which contains all
-- the defined variables and their value in a map.
evaluate :: Tree -> Evaluator Double

evaluate (SumNode op left right) = do
    lft <- evaluate left
    rgt <- evaluate right
    case op of
        Plus  -> return (lft + rgt)
        Minus -> return (lft - rgt)
    
evaluate (ProdNode op left right) = do
    lft <- evaluate left
    rgt <- evaluate right
    case op of
        Times -> return (lft * rgt)
        Div   -> return (lft / rgt)

evaluate (UnaryNode op tree) = do
    x <- evaluate tree
    case op of
        Plus -> return x
        Div  -> return (-x)

evaluate (NumNode x) = return x

evaluate (VarNode str) = lookUp str

-- The function addSymbol only returns an Evaluator to propagate the state.
-- Therefore the return function is then required afterward to evaluate the 
-- value.
evaluate (AssignNode str tree) = do
    v <- evaluate tree
    addSymbol str v
    return v

runEvaluator :: Evaluator a -> SymTab -> (a, SymTab)
runEvaluator ev s = runState ev s

initSymTab :: SymTab
initSymTab = M.fromList [("pi", pi), ("e", exp 1.0)]

