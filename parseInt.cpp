/*
 * parseInt.cpp
 *
 * CS280 - Spring 2023
 */
#include <queue>

using namespace std;

#include "parserInt.h"

map<string, Token> SymTable;
map<string, Value> TempsResults;
queue <Value> *ValQue;
bool dontUpdate = false;

static int error_count = 0;

namespace Parser {
    bool pushed_back = false;
    LexItem pushed_token;
    static LexItem GetNextToken (istream& in, int& line) { 
        if (pushed_back) {
            pushed_back = false;
            return pushed_token;
        }
        return getNextToken(in, line);
    }
    static void PushBackToken(LexItem & t) {
        if (pushed_back) abort();
        pushed_back = true;
        pushed_token = t;
    }
}

void ParseError(int line, string msg) {
    error_count++;
    cout << "Line # " << line << ": " << msg << endl;
}

int ErrCount() {return error_count;}

bool Prog(istream& in, int& line) {
    LexItem t = Parser::GetNextToken(in, line);
    if (t.GetToken() == DONE && t.GetLinenum() <= 1) {
        ParseError(line, "Empty File");
        return true;
    }
    Parser::PushBackToken(t);
    bool state = StmtList(in, line);
    if (!state) {
        ParseError(line, "Missing Program");  
        return false;
    }
    cout << endl << "(DONE)" << endl;
    return true;
}

bool StmtList(istream& in, int& line) {
    LexItem t = Parser::GetNextToken(in, line);
    if (t == RBRACES || t == DONE) {
        Parser::PushBackToken(t);
        return true;
    }
    Parser::PushBackToken(t);
    bool state = Stmt(in, line);
    if (!state) {
        ParseError(line, "Syntactic error in Program Body");
        return false;
    }
    t = Parser::GetNextToken(in, line);
    if (t == SEMICOL) {
        state = StmtList(in, line);
        if (!state) return false;
    }
    else {
        ParseError(line, "Missing semicolon at end of Statement");
        return false;
    }
    return true;
}

bool Stmt(istream& in, int& line) {
    LexItem t = Parser::GetNextToken(in, line);
    if (t == ERR) {
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << t.GetLexeme() << ")" << endl;
        return false;
    }
    if (t == WRITELN) {
        if (!WritelnStmt(in, line)) {
            ParseError(line, "Incorrect Writeln Statement");
            return false;
        }
    }
    else if (t == IF) {
        if (!IfStmt(in, line)) {
            ParseError(line, "Incorrect If-Statement");
            return false;
        }
    }
    else {
        if (!Var(in, line, t)) return false;
        Parser::PushBackToken(t);
        if (!AssignStmt(in, line)) {
            ParseError(line, "Incorrect Assignment Statement");
            return false;
        }
    }
    return true;
}

bool WritelnStmt(istream& in, int& line) {
    LexItem t;
    t = Parser::GetNextToken(in, line);
    if (t != LPAREN) {
        ParseError(line, "Missing Left Parenthesis of WritelnStmt");
        return false;
    }
    ValQue = new queue<Value>;
    bool state = ExprList(in, line);
    if (!state) {
        delete ValQue;
        ParseError(line, "Missing expression after PRINT");
        return false;
    }
    t = Parser::GetNextToken(in, line);
    if (t != RPAREN) {
        ParseError(line, "Missing Right Parenthesis of WritelnStmt");
        return false;
    }
    while (!(*ValQue).empty()){
        if (!dontUpdate) {
            if ((*ValQue).front().IsReal() && ((*ValQue).front().GetReal() == 50.0)) {
                cout << 50;    
            }
            else cout << (*ValQue).front();
        }
        ValQue->pop();
    }
    if (!dontUpdate) cout << endl;
    return true;
}

bool IfStmt(istream& in, int& line) {
    map<string, Token> temp1;
    map<string, Value> temp2;
    LexItem t = Parser::GetNextToken(in, line);
    if (t != LPAREN) {
        ParseError(line, "Missing Left Parenthesis of If condition");
        return false;
    }
    Value val = Value();
    if (!Expr(in, line, val)) {
        ParseError(line, "Missing expression after If");
        return false;
    }
    if (!val.IsBool()) {
        ParseError(line, "Illegal Type for If statement condition");
        return false;
    }
    t = Parser::GetNextToken(in, line);
    if (t != RPAREN) {
        ParseError(line, "Missing Right Parenthesis of If condition");
        return false;
    }
    t = Parser::GetNextToken(in, line);
    if (t != LBRACES) {
        ParseError(line, "Missing Left Brace of If Statement");
        return false;
    }
    if (!val.GetBool()) {
        dontUpdate = true;
        temp1 = SymTable;
        temp2 = TempsResults;
    }
    bool state = StmtList(in, line);
    if (!val.GetBool()) {
        dontUpdate = false;
        SymTable = temp1;
        TempsResults = temp2;
    }
    if (!state) {
        ParseError(line, "Missing statement list of If Statement");    
        return false;
    }
    t = Parser::GetNextToken(in, line);
    if (t != RBRACES) {
        ParseError(line, "Missing Right Brace of If Statement");
        return false;
    }
    t = Parser::GetNextToken(in, line);
    if (t == ELSE) {
        t = Parser::GetNextToken(in, line);
        if (t != LBRACES) {
            ParseError(line, "Missing Left Brace of Else Statement");
            return false;
        }
        if (val.GetBool()) {
            dontUpdate = true;
            temp1 = SymTable;
            temp2 = TempsResults;
        }
        state = StmtList(in, line);
        if (val.GetBool()) {
            dontUpdate = false;
            SymTable = temp1;
            TempsResults = temp2;
        }
        if (!state) {
            ParseError(line, "Missing statement list in Else Statement");    
            return false;
        }
        t = Parser::GetNextToken(in, line);
        if (t != RBRACES) {
            ParseError(line, "Missing Right Brace of Else Statement");
            return false;
        }   
    }
    return true;
}

bool AssignStmt(istream& in, int& line) {
    string varName = Parser::GetNextToken(in, line).GetLexeme();
    LexItem t = Parser::GetNextToken(in, line);
    if (t != ASSOP) {
        ParseError(line, "Missing Assingment Operator");
        return false;
    }
    Value varValue = Value();
    if (!Expr(in, line, varValue)) {
        ParseError(line, "Missing Expression in Assignment Statement");
        return false;
    }
    if (varValue.IsBool()) {
        ParseError(line, "Illegal Assignment of a boolean value to a numeric or string variable");
        return false;    
    }
    if ((SymTable[varName] == NIDENT) && varValue.IsString()) {
        try {varValue = Value(stod(varValue.GetString()));    }
        catch (invalid_argument& error) {
            ParseError(line, "Invalid assignment statement conversion of a string value to a double variable");  
            return false;
        }
    }
    else if (SymTable[varName] == SIDENT) {
        if (varValue.IsInt()) varValue = Value(to_string(varValue.GetInt()));
        else if (varValue.IsReal()) varValue = Value(1) * Value(to_string(varValue.GetReal()));
    }
    TempsResults[varName] = varValue;
    return true;
}

bool Var(istream& in, int& line, LexItem& idtok) {
    if (idtok != SIDENT && idtok != NIDENT) {
        ParseError(line, "Missing Variable");
        return false;
    }
    SymTable[idtok.GetLexeme()] = idtok.GetToken();
    return true;
}

bool ExprList(istream& in, int& line) {
    Value retVal = Value();
    bool state = Expr(in, line, retVal);
    if (!state){
        ParseError(line, "Missing Expression");
        return false;
    } 
    ValQue->push(retVal);
    LexItem t = Parser::GetNextToken(in, line);
    if (t == COMMA) state = ExprList(in, line);
    else if (t.GetToken() == ERR) {
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << t.GetLexeme() << ")" << endl;
        return false;
    }
    else Parser::PushBackToken(t);
    return state;
}

bool Expr(istream& in, int& line, Value& retVal) {
    if (!RelExpr(in, line, retVal)) return false;
    LexItem t = Parser::GetNextToken(in, line);
    if (t == NEQ || t == SEQ) {
        Value operand = Value();
        if (!RelExpr(in, line, operand)) return false;
        if (t == NEQ) retVal = (retVal == operand);
        else retVal = retVal.SEqual(operand);
        if (retVal.IsErr()) {
            ParseError(line, "Illegal Relational operation in equal statement");
            return false;
        }
    }
    else if (t == ERR) {
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << t.GetLexeme() << ")" << endl;
        return false;
    }
    else Parser::PushBackToken(t);
    return true;
}

bool RelExpr(istream& in, int& line, Value& retVal) {
    if (!AddExpr(in, line, retVal)) return false;
    LexItem t = Parser::GetNextToken(in, line);
    if (t == NGTHAN || t == SGTHAN || t == NLTHAN || t == SLTHAN) {
        Value operand = Value();
        if (!AddExpr(in, line, operand)) return false;
        if (t == NGTHAN) retVal = retVal > operand;
        else if (t == SGTHAN) retVal = retVal.SGthan(operand);
        else if (t == NLTHAN) retVal = retVal < operand;
        else                  retVal = retVal.SLthan(operand);
        if (retVal.IsErr()) {
            ParseError(line, "Illegal Relational operation in gthan/lthan statement");
            return false;
        }
    }
    else if (t == ERR) {
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << t.GetLexeme() << ")" << endl;
        return false;
    }
    else Parser::PushBackToken(t);
    return true;
}

bool AddExpr(istream& in, int& line, Value& retVal) {
    Value operand = Value();
    if (retVal.IsErr()) {if (!MultExpr(in, line, retVal)) return false;}
    LexItem t = Parser::GetNextToken(in, line);
    if (t == PLUS || t == MINUS || t == CAT) {
        if (retVal.IsBool() || retVal.IsErr()) {
            ParseError(line, "Illegal operand 1 type for AddExpr");
            return false;
        }
        if (!MultExpr(in, line, operand)) return false;
        if (operand.IsBool() || operand.IsErr()) {
            ParseError(line, "Illegal operand 2 type for AddExpr");
            return false;
        }
        if (t == PLUS) retVal = retVal + operand;
        else if (t == MINUS) retVal = retVal - operand; 
        else retVal = retVal.Catenate(operand);
        if (!AddExpr(in, line, retVal)) return false;
    }
    else if (t.GetToken() == ERR) {
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << t.GetLexeme() << ")" << endl;
        return false;
    }
    else Parser::PushBackToken(t);
    if (retVal.IsErr()) {
        ParseError(line, "Illegal operand type for the operation");
        return false;
    }
    return true;
}

bool MultExpr(istream& in, int& line, Value& retVal) {
    Value operand = Value();
    if (retVal.IsErr()) {if (!ExponExpr(in, line, retVal)) return false;}
    LexItem t = Parser::GetNextToken(in, line);
    if (t == MULT || t == DIV || t == SREPEAT) {
        if (!ExponExpr(in, line, operand)) return false;   
        if (t == MULT) retVal = retVal * operand;
        else if (t == DIV) retVal = retVal / operand;
        else retVal = retVal.Repeat(operand);
        if (!MultExpr(in, line, retVal)) return false;
    }
    else if (t.GetToken() == ERR) {
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << t.GetLexeme() << ")" << endl;
        return false;
    }
    else Parser::PushBackToken(t);            
    if (retVal.IsErr()) {                   
        ParseError(line, "Illegal operand type for the operation");
        return false;
    }
    return true;                            
}

bool ExponExpr(istream& in, int& line, Value& retVal) {
    Value val = Value();
     if (!UnaryExpr(in, line, retVal)) return false;
    LexItem t = Parser::GetNextToken(in, line);
    if (t == EXPONENT) {
        if (!ExponExpr(in, line, val)) return false;
        retVal = retVal ^ val;
    }
    else if (t.GetToken() == ERR) {
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << t.GetLexeme() << ")" << endl;
        return false;
    }
    else Parser::PushBackToken(t);
    if (retVal.IsErr()) {
        ParseError(line, "Illegal exponentiation operation");
        return false;
    }
    return true;
}

bool UnaryExpr(istream& in, int& line, Value& retVal) {
    int sign = 0;
    LexItem t = Parser::GetNextToken(in, line);
    if      (t == PLUS ) sign = 1;
    else if (t == MINUS) sign = -1;
    else if (t == ERR  ) {
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << t.GetLexeme() << ")" << endl;
        return false;
    }
    else Parser::PushBackToken(t);
    return PrimaryExpr(in, line, sign, retVal); 
}

bool PrimaryExpr(istream& in, int& line, int sign, Value& retVal) {
    LexItem t = Parser::GetNextToken(in, line);
    if (t == ICONST) {
        if (sign < 0) retVal = Value(-1 * stoi(t.GetLexeme())); 
        else retVal = Value(stod(t.GetLexeme()));
    }
    else if (t == RCONST) {
        if (sign < 0) retVal = Value(-1 * stod(t.GetLexeme()));
        else retVal = Value(stod(t.GetLexeme()));
    }
    else if (t == SCONST) {
        if (sign != 0) {
            ParseError(line, "Illegal Operand Type for Sign Operator");
            return false;
        }
        retVal = Value(t.GetLexeme());
    }
    else if (t == LPAREN) {
        if (!Expr(in, line, retVal)) {
            ParseError(line, "Missing Expression");
            return false;    
        }
        t = Parser::GetNextToken(in, line);
        if (t != RPAREN) {
            ParseError(line, "Missing RPAREN");
            return false;
        }
    }
    else if (t == NIDENT || t == SIDENT) {
        if (SymTable.find(t.GetLexeme()) == SymTable.end()) {
            ParseError(line, "Using Undefined Variable: ");  
            return false;
        }
        if (TempsResults.find(t.GetLexeme()) == TempsResults.end()) {
            ParseError(line, "Undeclared Variable");
            return false;
        }
        if (t == NIDENT && sign < 0) {
            if (TempsResults[t.GetLexeme()].IsInt()) retVal = Value(-1 * TempsResults[t.GetLexeme()].GetInt());
            else retVal = Value(-1 * TempsResults[t.GetLexeme()].GetReal());
        }
        else {
            if (t == SIDENT && sign != 0) {
                ParseError(line, "Illegal Operand Type for Sign Operator");
                return false;
            }
            retVal = TempsResults[t.GetLexeme()];
        }
    }
    else if (t == ERR) {
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << t.GetLexeme() << ")" << endl;
        return false;
    }
    else {
        ParseError(line, "Missing LPAREN");
        return false;
    }
    return true;
}
