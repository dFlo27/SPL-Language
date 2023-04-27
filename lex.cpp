#include <iostream>
#include <map>
#include "lex.h"

static const char * tokToStr[] = {
    "WRITELN", "IF", "ELSE", "IDENT", "NIDENT", "SIDENT",
	"ICONST", "RCONST", "SCONST", "PLUS", "MINUS", "MULT", 
    "DIV", "EXPONENT", "ASSOP", "LPAREN", "RPAREN", "LBRACES", "RBRACES", "NEQ", 
	"NGTHAN", "NLTHAN", "CAT", "SREPEAT", "SEQ", "SLTHAN", "SGTHAN", 
	"COMMA", "SEMICOL", "ERR", "DONE"};

map <string, Token> strToTok = {
		{ "+", PLUS },
		{ "/" , DIV },
		{ "^" , EXPONENT },
		{ ">" , NGTHAN  },
		{ "<", NLTHAN },
		{ ".", CAT },
		{ ",", COMMA  },
		{ "(", LPAREN },
		{ ")", RPAREN },
		{ "{", LBRACES },
		{ "}", RBRACES  },
		{ ";", SEMICOL  },
};
LexItem getNextToken(istream& in, int& linenum) {
    enum TokState {START, nident, sident, ident, iconst, sconst, writeln, Else, rconst, seq, sgthan, slthan}
    lexstate = START;
    string lexeme = "";
    string temp;
    char ch;
    while(in.get(ch)) {
        if (ch == '#') {
            getline(in, temp);
            if (temp.find('\n')) linenum += 1;
            continue;
        }
        if (ch == '\n') linenum += 1;
        if (lexstate == START && (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t')) continue;
        lexeme = lexeme + ch;
        switch (lexstate)  {
            case START:
                if (ch == '-') {
                    ch = in.get();
                    lexeme += ch;
                    if (ch == 'e' || ch == 'E')      lexstate = seq;
                    else if (ch == 'g' || ch == 'G') lexstate = sgthan;
                    else if (ch == 'l' || ch == 'L') lexstate = slthan;
                    else {
                        in.putback(ch);
                        return LexItem(MINUS, "-", linenum);
                    }
                    break;
                }
                if (ch == '=') {
                    if (in.peek() == '=') {
                        lexeme += in.get();
                        return LexItem(NEQ, lexeme, linenum);
                    }
                    return LexItem(ASSOP, lexeme, linenum);    
                }
                if (ch == '*' ) {
                    if (in.peek() == '*') {
                        lexeme += in.get();
                        return LexItem(SREPEAT, lexeme, linenum);
                    }
                    return LexItem(MULT, lexeme, linenum);
                }
                if (strToTok.find(lexeme) != strToTok.end()) return id_or_kw(lexeme, linenum);
                else if (ch == '@')   lexstate = sident;
                else if (ch == '$')   lexstate = nident;
                else if (isdigit(ch)) lexstate = iconst;
                else if (isalpha(ch) || ch == '_') {
                    if (ch == 'i' && in.peek() == 'f') {
                        lexeme += in.get();
                        return LexItem(IF, "if", linenum);   
                    }
                    else if (ch == 'w') lexstate = writeln;
                    else if (ch == 'e') lexstate = Else;
                    else lexstate = ident;   
                }
                else if (ch == '\"' || ch == '\'') lexstate = sconst;
                else return LexItem(ERR, lexeme, linenum);
                break;
            
            case nident:
            case sident:
            case ident:
                if (!isalpha(ch) && !isdigit(ch) && ch != '_') {
                   if (ch != '\n') in.putback(ch);
                   lexeme = lexeme.substr(0, lexeme.length() - 1);
                   return id_or_kw(lexeme, linenum);
                }  
                break;
                
            case iconst:
            case rconst:
                if (ch == '.') {
                    if (lexstate == rconst) return LexItem(ERR, lexeme, linenum);
                    else lexstate = rconst;
                }
                else if (!isdigit(ch)) {
                    if (ch != '\n') in.putback(ch);
                    lexeme = lexeme.substr(0, lexeme.length() - 1);
                    if (lexstate == iconst) return LexItem(ICONST, lexeme, linenum);
                    else return LexItem(RCONST, lexeme, linenum);
                }
                break;
            case sconst:
                if (ch == '\'' || ch == '\n') {
                    if (ch != lexeme[0]) {
                        if (ch == '\n') linenum -= 1;
                        return LexItem(ERR, lexeme.substr(0, lexeme.length() - 1), linenum);
                    }
                    else return LexItem(SCONST, lexeme.substr(1, lexeme.length() -2), linenum);
                }
                break;
                
            case writeln:
            case Else:
                if (lexstate == writeln) temp = "writeln";
                else temp = "else";
                if (ch != temp[lexeme.length()-1]) {
                    if (ch != '\n') in.putback(ch);
                    lexeme = lexeme.substr(0, lexeme.length() - 1);
                    lexstate = ident;  
                }
                else if (lexeme == temp) {
                    if (temp == "writeln") return LexItem(WRITELN, lexeme, linenum);    
                    else return LexItem(ELSE, lexeme, linenum);
                }
                break;
                
            case seq:
                if (ch == 'q' || ch == 'Q') return LexItem(SEQ, lexeme, linenum);
                else return LexItem(ERR, lexeme, linenum);   
            break;
                
            case sgthan:
            case slthan:
                if (ch == 't' || ch == 'T') return id_or_kw(lexeme, linenum);
                else return LexItem(ERR, lexeme, linenum);   
            break;
        }
    }
    return LexItem(DONE, "", linenum);
}
LexItem id_or_kw(const string& lexeme, int linenum) {
    if (strToTok.find(lexeme) != strToTok.end()) return LexItem(strToTok.find(lexeme)->second, lexeme, linenum);
    if (lexeme[0] == '@') return LexItem(SIDENT, lexeme, linenum);    
    if (lexeme[0] == '$') return LexItem(NIDENT, lexeme, linenum);    
    if (isalpha(lexeme[0]) || isdigit(lexeme[0]) || lexeme[0] == '_') return LexItem(IDENT, lexeme, linenum);    
    if (lexeme[0] == '-') {
        if (lexeme[1] == 'g' || lexeme[1] == 'G') return LexItem(SGTHAN, lexeme, linenum);
        else return LexItem(SLTHAN, lexeme, linenum);
    }
    return LexItem(ERR, lexeme, linenum);
}
ostream& operator<<(ostream& out, const LexItem& tok) {
    Token token = tok.GetToken();
    string lexeme = tok.GetLexeme();
    int lineNum = tok.GetLinenum();
    if (token == Token(ERR)) {
        cout << "Error in line " << lineNum << " (" << lexeme << ")";
        return out;
    }
    cout << tokToStr[token];    
    if (token == IDENT || token == NIDENT || token == SIDENT || token == SCONST || token == ICONST || token == RCONST) {
        cout << "(" << lexeme << ")";
    }
    return out;
}