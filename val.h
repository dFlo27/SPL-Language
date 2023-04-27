#ifndef VALUE_H
#define VALUE_H

#include <iostream>
#include <string>
#include <iomanip>
#include <stdexcept>
#include <regex>


using namespace std;

enum ValType {VINT, VREAL, VSTRING, VBOOL, VERR};

class Value {
    ValType	T;
    bool    Btemp;
	double  Rtemp;
    string  Stemp;
    int     Itemp;
       
public:
    Value()          : T(VERR)   , Btemp(false), Rtemp(0.0), Stemp(""), Itemp(0)  {}
    Value(bool vb)   : T(VBOOL)  , Btemp(vb)   , Rtemp(0.0), Stemp(""), Itemp(0)  {}
    Value(double vr) : T(VREAL)  , Btemp(false), Rtemp(vr) , Stemp(""), Itemp(0)  {}
    Value(string vs) : T(VSTRING), Btemp(false), Rtemp(0.0), Stemp(vs), Itemp(0)  {}
    Value(int vi)    : T(VINT)   , Btemp(false), Rtemp(0.0), Stemp(""), Itemp(vi) {}
    
    ValType GetType() const {return T;           }
    bool IsErr     () const {return T == VERR;   }
    bool IsString  () const {return T == VSTRING;}
    bool IsReal    () const {return T == VREAL;  }
    bool IsBool    () const {return T == VBOOL;  }
    bool IsInt     () const {return T == VINT;   }
    
    int    GetInt   () const {if (IsInt()   ) return Itemp; throw "RUNTIME ERROR: Value not an integer";}
    string GetString() const {if (IsString()) return Stemp; throw "RUNTIME ERROR: Value not a string";  }
    double GetReal  () const {if (IsReal()  ) return Rtemp; throw "RUNTIME ERROR: Value not a real";    }
    bool   GetBool  () const {if(IsBool()   ) return Btemp; throw "RUNTIME ERROR: Value not a boolean"; }
    
    void SetType  (ValType type) {T = type;   }
	void SetInt   (int val     ) {Itemp = val;}
	void SetReal  (double val  ) {Rtemp = val;}
	void SetString(string val  ) {Stemp = val;}
	void SetBool  (bool val    ) {Btemp = val;}
	
    //Returns a string str repeated n times
    string repeat(string str, int n) const {
        string result = "";
        for (int i = 0; i < n; i++) {result += str;}
        return result;
    }
    //Real (Double) to String, with no trailing zeros
    string rtos(double num) const {return regex_replace(to_string(num), regex("(\\.\\d*?)0+$"), "$1");}
    
    //For Numeric operations
    bool checkValue(Value val, double oper[2]) const {
        if (IsErr() || IsBool() || val.IsErr() || val.IsBool()) return false;
        if      (IsInt() ) oper[0] = GetInt();
        else if (IsReal()) oper[0] = GetReal();
        else {
            try {oper[0] = stod(GetString());}
            catch (invalid_argument& error) {
                cerr << "Invalid conversion from string to double." << endl;
                return false;
            }
        }
        if      (val.IsInt() ) oper[1] = val.GetInt();
        else if (val.IsReal()) oper[1] = val.GetReal();
        else {
            try {oper[1] = stod(val.GetString());}
            catch (invalid_argument& error) {
                cerr << "Invalid conversion from string to double." << endl;
                return false;
            }
        }    
        return true;
    }
    
    //For String operations
    bool checkValue(Value val, string oper[2]) const {
        if (IsErr() || IsBool() || val.IsErr() || val.IsBool()) return false;
        if (IsInt()) oper[0] = to_string(GetInt());
        else if (IsReal()) {
            if (GetReal() == (int) GetReal()) oper[0] = to_string((int) GetReal());
            else oper[0] = rtos(GetReal());
        }
        else oper[0] = GetString();
        if (val.IsInt()) oper[1] = to_string(val.GetInt());
        else if (val.IsReal()) {
            if (val.GetReal() == (int) val.GetReal()) oper[1] = to_string((int) val.GetReal());
            else oper[1] = rtos(val.GetReal());
        }
        else oper[1] = val.GetString();
        return true;
    }
    
    // numeric overloaded add op to this
    Value operator+(const Value& op) const {
        double result[2];
        if (checkValue(op, result)) return Value(result[0] + result[1]);
        return Value();
    }
    
    // numeric overloaded subtract op from this
    Value operator-(const Value& op) const {
        double result[2];
        if (checkValue(op, result)) return Value(result[0] - result[1]);
        return Value();
    }
    
    // numeric overloaded multiply this by op
    Value operator*(const Value& op) const {
        double result[2];
        if (checkValue(op, result)) return Value(result[0] * result[1]);
        return Value();
    }
    
    // numeric overloaded divide this by op
    Value operator/(const Value& op) const {
        double result[2];
        if (checkValue(op, result)) {
            if (result[1] == 0) cerr << "Run-Time Error-Illegal Division by Zero" << endl;
            else return Value(result[0] / result[1]);
        }
        return Value();
    }
    //numeric overloaded equality operator of this with op
    Value operator==(const Value& op) const {
        double result[2];
        if (checkValue(op, result)) return result[0] == result[1]? Value(true) : Value(false);
        return Value();
    }
	//numeric overloaded greater than operator of this with op
	Value operator>(const Value& op) const {
        double result[2];
        if (checkValue(op, result)) return result[0] > result[1]? Value(true) : Value(false);
        return Value();
    }
	//numeric overloaded less than operator of this with op
	Value operator<(const Value& op) const {
        double result[2];
        if (checkValue(op, result)) return result[0] < result[1]? Value(true) : Value(false);
        return Value();
    }
	
	//Numeric exponentiation operation this raised to the power of op 
	Value operator^(const Value& oper) const {
        double op1, op2;
        if (IsInt()) op1 = GetInt();
        else if (IsReal()) op1 = GetReal();
        else return Value();
        if (oper.IsInt()) op2 = oper.GetInt();
        else if (oper.IsReal()) op2 = oper.GetReal();
        else return Value();
        return Value(pow(op1, op2));
    }
	//string concatenation operation of this with op
	Value Catenate(const Value& oper) const {
        string result[2];
        if (checkValue(oper, result)) return Value(result[0] + result[1]);
        return Value();
    }    
	//string repetition operation of this with op
	Value Repeat(const Value& oper) const {
        if (IsErr() || IsBool() || oper.IsErr() || oper.IsBool()) return Value();
        double num;
        if (IsString()) {
            if (oper.IsInt()) return Value(repeat(GetString(), oper.GetInt()));
            if (oper.IsReal()) return Value(repeat(GetString(), (int) oper.GetReal()));
            try {num = stod(oper.GetString());}
            catch (invalid_argument& error) {
                cout << "Invalid conversion from string to double." << endl;
                return Value();
            }
            return Value(repeat(GetString(), num));
        }
        if (IsInt()) {
            if (oper.IsString()) return Value(repeat(oper.GetString(), GetInt()));
            return Value();
        }
        if (IsReal()) {
            if (oper.IsString()) return Value(repeat(oper.GetString(), (int) GetReal()));
            return Value();
        }
        return Value();
    }
	//string equality (-eq) operator of this with op
	Value SEqual(const Value& oper) const {
        string result[2];
        if (checkValue(oper, result)) return Value(result[0] == result[1]);
        return Value();
    }
	//string greater than (-gt) operator of this with op
	Value SGthan(const Value& oper) const {
        string result[2];
        if (checkValue(oper, result)) return result[0] > result[1]? Value(true) : Value(false);
        return Value();
    }
	//String less than operator of this with op
	Value SLthan(const Value& oper) const { 
        string result[2];
        if (checkValue(oper, result)) return result[0] < result[1]? Value(true) : Value(false);
        return Value();
    }
	    
    friend ostream& operator<<(ostream& out, const Value& op) {
        if      (op.IsInt())    out << op.Itemp;
		else if (op.IsString()) out << op.Stemp;
        else if (op.IsReal()  ) out << fixed << showpoint << setprecision(1) << op.Rtemp;
        else if (op.IsBool()  ) out << (op.GetBool()? "true" : "false");
        else    out << "ERROR";
        return  out;
    }
};


#endif
