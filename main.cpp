/*
Copyright 2022-23 Wolfgang Hermsen

This file is part of the SOPL Interpreter.

    The SOPL Interpreter is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The SOPL Interpreter is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with the SOPL Interpreter.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <iostream>
#include <cstdio>
#include <string>
#include <fstream>
#include <vector>
#include "Lexer.h"
#include "Parser.h"
#include "Enums.h"
#include "Log.h"
#include "Second.h"
#include "Third.h"
#include "Interpreter.h"
#include "Basics.h"
#include "Preprocess.h"
#include <algorithm>
#include "globals.h"
#include <stdlib.h>

using namespace std;


// In C++ non-const static members must be both declared in the class definition
// and defined with global scope to properly give the linker something to reference.
// NOTE: functions with side effects end with a digit (=no. of changed parameter)
//

string Log::filename = "";
Level Log::level = DEBUG;
// L = debug output for lexer
// P = debug output for parser
// I = debug output for interpreter
// b = debug output for plist
// c = debug output for paragraph context
// i = debug output for Item (during parse)
// j = debug output for jump calculation
// r = debug output for references
// s = debug output for sentence execution
// t = timing output for plist/pexpr
// x = debug output for indexes
// y = debug output for setting stop level

string debugtypes = ""; // always set debug types per -D
bool Log::useconsole = true;
string current_version = "1.0.0";

int main(int argc, char *argv[])
{

   // should switch from 850 to utf8  on windows
    #ifdef _WIN32
    system("chcp 65001 > NUL 2>&1");
    #endif // _WIN32

    Basics b;

    string filename = "";
    string original = "";
    string workDirectory = b.getDir(argv[0]);

    Lexer lx;
    Parser ps;
    Second p2;
    Third p3;
    Log log;

    log.setFile("Logfile.log");
    log.setLevel(WARN);
    int ix = 0;
    vector<string> v;
    vector<string> pargs;
    bool logOutput = false;
    bool foundFile = false;
    int retcode = 0;

    if (argc <= 2) {
      cout << "option or program missing"  << endl;
    } else {
        // read args
       for (int i = 1; i < argc; ++i) {
        string sargv = argv[i];
        if (foundFile) {
            pargs.push_back(sargv);
        } else if (sargv == "-r") {
            ix = 0;
        } else if(sargv == "-l") {
            ix = 1;
        } else if (sargv == "-p") {
            ix = 2;
        } else if (sargv == "-x") {
            ix = 3;
        } else if (sargv == "-L") {
            logOutput = true;
        } else if  (sargv.substr(0,2) == "-d") {
            log.setLevel(DEBUG);
            debugtypes = sargv.substr(2);
            debug_L = (debugtypes.find("L") != string::npos);
            debug_P = (debugtypes.find("P") != string::npos);
            debug_I = (debugtypes.find("I") != string::npos);
            debug_b = (debugtypes.find("b") != string::npos);
            debug_c = (debugtypes.find("c") != string::npos);
            debug_i = (debugtypes.find("i") != string::npos);
            debug_j = (debugtypes.find("j") != string::npos);
            debug_r = (debugtypes.find("r") != string::npos);
            debug_s = (debugtypes.find("s") != string::npos);
            debug_t = (debugtypes.find("t") != string::npos);
            debug_x = (debugtypes.find("x") != string::npos);
            debug_y = (debugtypes.find("y") != string::npos);
        } else if  (sargv == "-i") {
            log.setLevel(INFO);
        } else if  (sargv == "-a") {
            log.setLevel(WARNALL);
        } else if  (sargv == "-w") {
            log.setLevel(WARN);
        } else if  (sargv == "-e") {
            log.setLevel(ERROR);
        } else if  (sargv == "-f") {
            log.setLevel(FATAL);
        } else if  (sargv == "-t") {
            log.setLevel(TOTAL);
        } else if  (sargv == "-n") {
            log.setUseConsole(false);
        } else if  (sargv == "-ext") {
            isExtended = true;
        } else if  (sargv == "-std") {
            isExtended = false;
        } else if (sargv[0] == '-') {
            log.msg(WARN, "Unknown option " + sargv);
        } else {
            filename = sargv;
            original = sargv;
            foundFile = true;
        }
       }

       log.msg(INFO, "current version is " + current_version);

       if (!b.existsFile(filename)) {
            log.msg(FATAL, "file to be executed doesn't exist: " + filename);
            return -1;
       }

       string currDirectory = b.getDir(filename);
       Interpreter ip = Interpreter(workDirectory, currDirectory, logOutput);
       Preprocess pp = Preprocess();

       // do it
        if (ix >=0) {
          pp.setFile(filename);
          retcode = pp.process();
            if (retcode == 0) {
                v = pp.getComplete();
                if (ix==0) {if (debug_L) {log.debug("file line count is " +  to_string(v.size()));};}
            }
        };

        if ((ix >= 1) && (retcode == 0)) {
          lx.setContent(v);
          if (debug_L) {log.debug("Symbols " + lx.getSymbolsString());};
        };

        if ((ix >=2)  && (retcode == 0)){
          retcode = ps.setParse(lx.getSymbols());
          if (debug_P) {log.debug("Parser result is\n" + ps.getParserString());};
          if (retcode >= 0) {
            retcode = p2.secondParse(ps.getParse());
            if (debug_P) {log.debug("Second parse is\n" + p2.getContentString());};
            if (retcode >= 0) {
                retcode = p3.thirdParse(p2.getSentences());
                if (debug_P) {log.debug("Third parse: " + p3.getContentString());};
            }
          }
        };

        if (retcode < 0) {
            log.msg(ERROR, "no execution because parsing failed.");
        } else if (ix >=3) {
           ip.setParagraphs(p3.getParagraphs());
           if (debug_I) {log.debug("Interpreter: " + ip.getNameList());};
           ip.execute(pargs);
        };
   }

   return 0;
}

