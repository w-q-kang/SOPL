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

#include "globals.h"
using namespace std;

bool isExtended = false;
 vector<string> refs = {"this", "params", "forparams", "useparams", "withparams", "ref", "args"};
 vector<string>  gwords = {"for", "use", "with"};
        // standard verbs, must not contain: plist, between, nop, stop
 vector<string> standard = {"cond", "del", "expand", "file", "find", "freq", "get", "id", "include", "input", "ins", "join", "mask", "minus",
            "output", "pexpr", "print", "range", "readLines", "reverse", "returnValue", "set", "sort", "split", "time", "unique", "writeLines"};
 vector<string> other = {"plist", "between", "nop", "stop"};
 vector<string> vmods = {"combine", "echo", "echoIn", "echoOut", "perRow", "int", "float", "forget"};
bool debug_L = false;
bool debug_P = false;
bool debug_I = false;
bool debug_b = false;
bool debug_c = false;
bool debug_i = false;
bool debug_j = false;
bool debug_r = false;
bool debug_s = false;
bool debug_t = false;
bool debug_x = false;
bool debug_y = false;
