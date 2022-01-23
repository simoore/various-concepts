module Main where

import System.IO (hFlush, stdout)
import Lexer (tokenize)
import Parser (parse)
import Evaluator (SymTab, evaluate, runEvaluator, initSymTab)

main :: IO ()
main = loop initSymTab

loop :: SymTab -> IO ()
loop symTab = do
    str <- prompt ">"
    if null str
    then
        return ()
    else
        let toks = tokenize str
            tree = parse toks
            act = evaluate tree
            (val, symTab') = runEvaluator act symTab
        in do
            print val
            loop symTab'

prompt :: String -> IO String
prompt text = do
    putStr text
    hFlush stdout
    getLine

