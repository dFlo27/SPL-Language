/* Simple Program Language
   David Florim
   This is the main file; 
   It reads any files given when called */

#include<iostream>
#include<fstream>
#include "parseInt.h"

using namespace std;

int main(int argc, char *argv[]) {
  int lineNumber = 1;
	istream *in = NULL;
	ifstream file;
  
	for (int i = 1; i < argc; i++) {
		string arg = argv[i];
		
		if (in != NULL) {
			cerr << "ONLY ONE FILE NAME ALLOWED" << endl;
			return 0;
		}
		else {
			file.open(arg.c_str());
			if (file.is_open() == false) {
				cerr << "CANNOT OPEN " << arg << endl;
				return 0;
			}
			in = &file;
		}
  }
  if(argc == 1) {
		cerr << "Missing File Name." << endl;
    return 0;
  }
  if (!Prog(*in, lineNumber)) cout << "\nUnsuccessful Interpretation " << endl << "Number of Errors " << ErrCount()  << endl;
	else cout << "\nSuccessful Execution" << endl;
}
